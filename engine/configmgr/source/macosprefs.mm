/* -*- Mode: ObjC++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <vector>

#include <premac.h>
#include <Foundation/Foundation.h>
#include <postmac.h>

#include <cpo/uno/RuntimeException.hpp>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include "macosprefs.hxx"
#include "writemodfile.hxx"

namespace configmgr
{
namespace
{
// The macOS counterpart of winreg.cxx: configuration settings are read from the preferences of
// the application (its bundle identifier is the preference domain), which is where an MDM
// configuration profile with a matching PayloadType puts them. Preferences can also be written
// with "defaults write <bundle id> ..." for testing, but only values delivered by a profile are
// "forced" (managed), and only those can lock a setting with Final.
//
// Every preference key that starts with '/' is a configuration path whose last segment is the
// property name; all other keys in the domain are ignored. The value of such a key is either
//
//  - a plain value: string, boolean, integer, real or data (written as hexBinary). It is parsed
//    with the type the property has in the schema, so no type needs to be given.
//  - an array of plain values: the items of a list property.
//  - a dictionary, which mirrors the value names of the Windows registry layer:
//      Value            the plain value or array, as above
//      Type             the oor:type to use, e.g. xs:boolean or oor:string-list; needed only for
//                       properties of type "any" and for new properties of extensible groups
//      Final            true to lock the setting (ignored for values not delivered by a profile)
//      Nil              true to set the property to nil (<value xsi:nil="true"/>)
//      External         true if the value names an external backend ("<backend> <value>")
//      ExternalBackend  the external backend, prepended to the value when External is true
//
// A path segment of the form Name#op (op being modify, replace, fuse or remove) becomes a
// <node oor:name="Name" oor:op="op"> element, as does every segment after it. That is how set
// members are created. Set-member segments such as Tmpl['Name'] are passed through unchanged.
//
// For example the preference
//   "/org.openoffice.UserProfile/Data/o" = { Value = "Example Corp."; Final = 1; }
// becomes the following in configuration:
//   <item oor:path="/org.openoffice.UserProfile/Data">
//       <prop oor:name="o" oor:finalized="true">
//           <value>Example Corp.</value>
//       </prop>
//   </item>
//
// A list property:
//   "/org.openoffice.Office.Common/Security/Scripting/SecureURL" = ( "file:///opt/corp/macros" )
// becomes
//   <item oor:path="/org.openoffice.Office.Common/Security/Scripting">
//       <prop oor:name="SecureURL">
//           <value><it>file:///opt/corp/macros</it></value>
//       </prop>
//   </item>
//
// A set member, created with #fuse:
//   "/org.openoffice.Office.OptionsDialog/OptionsDialogGroups/ProductName#fuse/Pages/Java#fuse/Hide"
//       = "true"
// becomes
//   <item oor:path="/org.openoffice.Office.OptionsDialog/OptionsDialogGroups">
//       <node oor:name="ProductName" oor:op="fuse">
//           <node oor:name="Pages">
//               <node oor:name="Java" oor:op="fuse">
//                   <prop oor:name="Hide">
//                       <value>true</value>
//                   </prop>
//               </node>
//           </node>
//       </node>
//   </item>
//
// A property of an extensible group, which needs a type:
//   "/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/AutoCheckEnabled"
//       = { Value = 0; Type = "xs:boolean"; Final = 1; }
// becomes
//   <item oor:path="/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments">
//       <prop oor:name="AutoCheckEnabled" oor:type="xs:boolean" oor:finalized="true">
//           <value>false</value>
//       </prop>
//   </item>
//
// A nil value (which differs from an empty one):
//   "/org.openoffice.Office.Common/Path/Current/Work" = { Nil = 1; Final = 1; }
// becomes
//   <item oor:path="/org.openoffice.Office.Common/Path/Current">
//       <prop oor:name="Work" oor:finalized="true">
//           <value xsi:nil="true"/>
//       </prop>
//   </item>

OUString toOUString(CFStringRef pString)
{
    if (pString == nullptr)
        return OUString();
    CFIndex nLength = CFStringGetLength(pString);
    if (nLength == 0)
        return OUString();
    std::vector<UniChar> aBuffer(nLength);
    CFStringGetCharacters(pString, CFRangeMake(0, nLength), aBuffer.data());
    return OUString(reinterpret_cast<sal_Unicode const*>(aBuffer.data()), nLength);
}

// Converts a plain property-list value (boolean, number, string or data) to the text that goes
// into <value>. Returns false for anything else.
bool scalarToText(CFTypeRef pValue, OUString& rText)
{
    if (pValue == nullptr)
        return false;
    CFTypeID nType = CFGetTypeID(pValue);
    // CFBoolean is not a CFNumber, and must be checked first: an NSNumber created from a BOOL
    // is bridged to one.
    if (nType == CFBooleanGetTypeID())
    {
        rText = CFBooleanGetValue(static_cast<CFBooleanRef>(pValue)) ? u"true"_ustr : u"false"_ustr;
        return true;
    }
    if (nType == CFNumberGetTypeID())
    {
        CFNumberRef pNumber = static_cast<CFNumberRef>(pValue);
        if (CFNumberIsFloatType(pNumber))
        {
            double fValue = 0;
            CFNumberGetValue(pNumber, kCFNumberDoubleType, &fValue);
            rText = OUString::number(fValue);
        }
        else
        {
            sal_Int64 nValue = 0;
            CFNumberGetValue(pNumber, kCFNumberSInt64Type, &nValue);
            rText = OUString::number(nValue);
        }
        return true;
    }
    if (nType == CFStringGetTypeID())
    {
        rText = toOUString(static_cast<CFStringRef>(pValue));
        return true;
    }
    if (nType == CFDataGetTypeID())
    {
        CFDataRef pData = static_cast<CFDataRef>(pValue);
        CFIndex nLength = CFDataGetLength(pData);
        UInt8 const* pBytes = CFDataGetBytePtr(pData);
        static constexpr char aDigits[] = "0123456789ABCDEF";
        OUStringBuffer aBuffer(nLength * 2);
        for (CFIndex i = 0; i < nLength; ++i)
        {
            aBuffer.append(
                OUStringChar(aDigits[pBytes[i] >> 4]) + OUStringChar(aDigits[pBytes[i] & 0xF]));
        }
        rText = aBuffer.makeStringAndClear();
        return true;
    }
    return false;
}

// A flag (Final, Nil, External) is set by a boolean true, the number 1, or the strings "1" and
// "true" (which is what "defaults write ... -dict" can express).
bool isFlagSet(CFTypeRef pValue)
{
    if (pValue == nullptr)
        return false;
    CFTypeID nType = CFGetTypeID(pValue);
    if (nType == CFBooleanGetTypeID())
        return CFBooleanGetValue(static_cast<CFBooleanRef>(pValue));
    if (nType == CFNumberGetTypeID())
    {
        sal_Int64 nValue = 0;
        CFNumberGetValue(static_cast<CFNumberRef>(pValue), kCFNumberSInt64Type, &nValue);
        return nValue == 1;
    }
    if (nType == CFStringGetTypeID())
    {
        OUString aText(toOUString(static_cast<CFStringRef>(pValue)));
        return aText == "1" || aText.equalsIgnoreAsciiCase("true");
    }
    return false;
}

OUString dictionaryString(CFDictionaryRef pDict, CFStringRef pKey)
{
    CFTypeRef pValue = CFDictionaryGetValue(pDict, pKey);
    if (pValue == nullptr || CFGetTypeID(pValue) != CFStringGetTypeID())
        return OUString();
    return toOUString(static_cast<CFStringRef>(pValue));
}

// Splits a path (without its leading '/') at the '/' characters that are not inside the [...]
// of a set-member segment.
std::vector<OUString> splitPath(std::u16string_view aPath)
{
    std::vector<OUString> aSegments;
    std::size_t nStart = 0;
    int nDepth = 0;
    for (std::size_t i = 0; i < aPath.size(); ++i)
    {
        if (aPath[i] == '[')
            ++nDepth;
        else if (aPath[i] == ']')
        {
            if (nDepth > 0)
                --nDepth;
        }
        else if (aPath[i] == '/' && nDepth == 0)
        {
            aSegments.emplace_back(aPath.substr(nStart, i - nStart));
            nStart = i + 1;
        }
    }
    aSegments.emplace_back(aPath.substr(nStart));
    return aSegments;
}

// Position of the '#' that separates a node operation from the node name, or npos.
std::size_t operationSeparator(std::u16string_view rSegment)
{
    std::size_t nHash = rSegment.rfind('#');
    std::size_t bracket = rSegment.rfind(']');
    if (nHash != std::u16string_view::npos && bracket != std::u16string_view::npos
        && nHash < bracket)
    {
        return std::u16string_view::npos; // part of a set-member name
    }
    return nHash;
}

bool isOperation(std::u16string_view aOperation)
{
    return aOperation == u"modify" || aOperation == u"replace" || aOperation == u"fuse"
           || aOperation == u"remove";
}

void writeItem(OUString const& rKey, CFTypeRef pValue, bool bForced, TempFile& rOut)
{
    std::vector<OUString> aSegments(splitPath(rKey.subView(1)));
    if (aSegments.size() < 2)
    {
        SAL_WARN("configmgr", "macosprefs: ignoring \"" << rKey << "\": no property name");
        return;
    }
    for (std::size_t i = 0; i < aSegments.size(); ++i)
    {
        OUString const& rSegment = aSegments[i];
        std::size_t nHash = i + 1 < aSegments.size()
            ? operationSeparator(rSegment) : std::u16string_view::npos;
        if (rSegment.isEmpty() || nHash == 0
            || (nHash != std::u16string_view::npos && !isOperation(rSegment.subView(nHash + 1))))
        {
            SAL_WARN("configmgr", "macosprefs: ignoring \"" << rKey << "\": bad path segment \""
                                                            << rSegment << "\"");
            return;
        }
    }

    bool bFinal = false;
    bool bNil = false;
    bool bExternal = false;
    OUString aType;
    OUString aExternalBackend;
    CFTypeRef pRawValue = pValue;
    if (pValue != nullptr && CFGetTypeID(pValue) == CFDictionaryGetTypeID())
    {
        CFDictionaryRef pDict = static_cast<CFDictionaryRef>(pValue);
        pRawValue = CFDictionaryGetValue(pDict, CFSTR("Value"));
        aType = dictionaryString(pDict, CFSTR("Type"));
        aExternalBackend = dictionaryString(pDict, CFSTR("ExternalBackend"));
        bFinal = isFlagSet(CFDictionaryGetValue(pDict, CFSTR("Final")));
        bNil = isFlagSet(CFDictionaryGetValue(pDict, CFSTR("Nil")));
        bExternal = isFlagSet(CFDictionaryGetValue(pDict, CFSTR("External")));
        if (bFinal && !bForced)
        {
            SAL_INFO("configmgr",
                     "macosprefs: \"" << rKey << "\" is not a managed preference, ignoring Final");
            bFinal = false;
        }
    }

    OUString aValue;
    std::vector<OUString> aItems;
    bool bList = false;
    if (pRawValue == nullptr)
    {
        if (!bNil)
        {
            SAL_WARN("configmgr", "macosprefs: ignoring \"" << rKey << "\": no Value");
            return;
        }
    }
    else if (CFGetTypeID(pRawValue) == CFArrayGetTypeID())
    {
        if (bNil || bExternal)
        {
            SAL_WARN("configmgr",
                     "macosprefs: ignoring \"" << rKey << "\": a list cannot be Nil or External");
            return;
        }
        bList = true;
        CFArrayRef pArray = static_cast<CFArrayRef>(pRawValue);
        CFIndex nCount = CFArrayGetCount(pArray);
        for (CFIndex i = 0; i < nCount; ++i)
        {
            OUString aItem;
            if (!scalarToText(CFArrayGetValueAtIndex(pArray, i), aItem))
            {
                SAL_WARN("configmgr",
                         "macosprefs: ignoring \"" << rKey << "\": unsupported list item type");
                return;
            }
            aItems.push_back(aItem);
        }
    }
    else if (!scalarToText(pRawValue, aValue))
    {
        SAL_WARN("configmgr", "macosprefs: ignoring \"" << rKey << "\": unsupported value type");
        return;
    }

    if (bExternal)
    {
        // type and external are mutually exclusive
        aType.clear();
        // Prepend backend, like in
        // "com.sun.star.configuration.backend.LdapUserProfileBe company"
        if (!aExternalBackend.isEmpty())
            aValue = aExternalBackend + " " + aValue;
    }

    SAL_INFO("configmgr", "macosprefs \"" << rKey << "\": value=\"" << aValue
                                          << "\" items=" << aItems.size() << " type=\"" << aType
                                          << "\" final=" << bFinal << " nil=" << bNil
                                          << " ext=" << bExternal << " forced=" << bForced);

    // The <item> is written like winreg.cxx does it: the opening quote of each attribute is
    // closed by whatever comes next.
    bool bHasNode = false;
    sal_Int32 nCloseNode = 0;
    rOut.writeString("<item oor:path=\"");
    for (std::size_t i = 0; i + 1 < aSegments.size(); ++i)
    {
        OUString const& rSegment = aSegments[i];
        std::size_t nHash = operationSeparator(rSegment);
        if (nHash != std::u16string_view::npos || bHasNode)
        {
            bHasNode = true;
            ++nCloseNode;
            rOut.writeString("\"><node oor:name=\"");
            if (nHash != std::u16string_view::npos)
            {
                writeAttributeValue(rOut, rSegment.subView(0, nHash));
                rOut.writeString("\" oor:op=\"");
                writeAttributeValue(rOut, rSegment.subView(nHash + 1));
            }
            else
            {
                writeAttributeValue(rOut, rSegment);
            }
        }
        else
        {
            writeAttributeValue(rOut, Concat2View("/" + rSegment));
        }
    }
    rOut.writeString("\">");

    rOut.writeString("<prop oor:name=\"");
    writeAttributeValue(rOut, aSegments.back());
    rOut.writeString("\"");
    if (!aType.isEmpty())
    {
        rOut.writeString(" oor:type=\"");
        writeAttributeValue(rOut, aType);
        rOut.writeString("\"");
    }
    if (bFinal)
        rOut.writeString(" oor:finalized=\"true\"");
    rOut.writeString("><value");
    if (bNil && !bList && aValue.isEmpty())
    {
        rOut.writeString(" xsi:nil=\"true\"/");
    }
    else if (bExternal)
    {
        rOut.writeString(" oor:external=\"");
        writeAttributeValue(rOut, aValue);
        rOut.writeString("\"/");
    }
    else
    {
        rOut.writeString(">");
        if (bList)
        {
            for (OUString const& rItem : aItems)
            {
                rOut.writeString("<it>");
                writeValueContent(rOut, rItem);
                rOut.writeString("</it>");
            }
        }
        else
        {
            writeValueContent(rOut, aValue);
        }
        rOut.writeString("</value");
    }
    rOut.writeString("></prop>");
    for (; nCloseNode > 0; nCloseNode--)
        rOut.writeString("</node>");
    rOut.writeString("</item>\n");
}
}

void writeMacOSPreferencesXcu(CFDictionaryRef prefs, CFSetRef forcedKeys, TempFile& out)
{
    CFIndex nCount = prefs == nullptr ? 0 : CFDictionaryGetCount(prefs);
    if (nCount == 0)
        return;
    std::vector<void const*> aKeys(nCount);
    std::vector<void const*> aValues(nCount);
    CFDictionaryGetKeysAndValues(prefs, aKeys.data(), aValues.data());

    std::vector<std::tuple<OUString, CFStringRef, CFTypeRef>> aEntries;
    for (CFIndex i = 0; i < nCount; ++i)
    {
        CFTypeRef pKey = aKeys[i];
        if (pKey == nullptr || CFGetTypeID(pKey) != CFStringGetTypeID())
            continue;
        CFStringRef pKeyString = static_cast<CFStringRef>(pKey);
        OUString aKey(toOUString(pKeyString));
        if (!aKey.startsWith("/"))
            continue;
        aEntries.emplace_back(aKey, pKeyString, aValues[i]);
    }
    std::sort(aEntries.begin(), aEntries.end(),
              [](auto const& a, auto const& b) { return std::get<0>(a) < std::get<0>(b); });

    for (auto const & [ rKey, pKeyString, pValue ] : aEntries)
    {
        bool bForced = forcedKeys != nullptr && CFSetContainsValue(forcedKeys, pKeyString);
        writeItem(rKey, pValue, bForced, out);
    }
}

bool dumpMacOSPreferences(OUString* pFileURL)
{
    @autoreleasepool
    {
        NSUserDefaults* pDefaults = [NSUserDefaults standardUserDefaults];
        // The union of the whole search list, with managed (profile) preferences on top; the
        // managed ones are not listed by CFPreferencesCopyKeyList.
        NSDictionary* pAll = [pDefaults dictionaryRepresentation];
        NSMutableDictionary* pPrefs = [NSMutableDictionary dictionary];
        NSMutableSet* pForced = [NSMutableSet set];
        for (id pKey in pAll)
        {
            if (![pKey isKindOfClass:[NSString class]] || ![pKey hasPrefix:@"/"])
                continue;
            pPrefs[pKey] = pAll[pKey];
            if ([pDefaults objectIsForcedForKey:pKey])
                [pForced addObject:pKey];
        }

        NSString* pBundleId = [[NSBundle mainBundle] bundleIdentifier];
        char const* pDomain = pBundleId != nil ? [pBundleId UTF8String] : "<no bundle identifier>";
        if ([pPrefs count] == 0)
        {
            SAL_INFO("configmgr",
                     "macosprefs: no configuration keys in the preferences of " << pDomain);
            return false;
        }
        SAL_INFO("configmgr", "macosprefs: " << [pPrefs count] << " configuration keys (" <<
                                  [pForced count] << " managed) in the preferences of " << pDomain);

        TempFile aFileHandle;
        switch (osl::FileBase::createTempFile(nullptr, &aFileHandle.handle, pFileURL))
        {
            case osl::FileBase::E_None:
                break;
            case osl::FileBase::E_ACCES:
                SAL_INFO("configmgr", "cannot create temp macOS preferences dump (E_ACCES)");
                return false;
            default:
                throw cpo::uno::RuntimeException(u"cannot create temporary file"_ustr);
        }
        aFileHandle.url = *pFileURL;
        aFileHandle.writeString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<oor:items"
                                " xmlns:oor=\"http://openoffice.org/2001/registry\""
                                " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
                                " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
        writeMacOSPreferencesXcu(reinterpret_cast<CFDictionaryRef>(pPrefs),
                                 reinterpret_cast<CFSetRef>(pForced), aFileHandle);
        aFileHandle.writeString("</oor:items>");
        oslFileError e = aFileHandle.closeWithoutUnlink();
        if (e != osl_File_E_None)
            SAL_WARN("configmgr", "osl_closeFile failed with " << +e);
        return true;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
