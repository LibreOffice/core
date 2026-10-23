/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include <test/unoapi_test.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>

#include <filesystem>
#include <array>

using namespace css;
using namespace css::awt;
using namespace css::beans;
using namespace css::container;
using namespace css::embed;
using namespace css::lang;
using namespace css::ui;
using namespace css::util;
using namespace ::cpo;
using namespace ::cpo::uno;

namespace
{
struct KeyIdentifierInfo
{
    OUString sIdentifier;
    sal_Int16 nCode;
};

using IdentifierHashMap = std::map<OUString, sal_Int16>;

using CodeHashMap = std::map<sal_Int16, OUString>;

class KeyMapping
{
    IdentifierHashMap aIdentifierHashMap;
    CodeHashMap aCodeHashMap;

public:
    KeyMapping()
    {
        KeyIdentifierInfo aInfoMap[]{ { u"0"_ustr, css::awt::Key::NUM0 },
                                      { u"1"_ustr, css::awt::Key::NUM1 },
                                      { u"2"_ustr, css::awt::Key::NUM2 },
                                      { u"3"_ustr, css::awt::Key::NUM3 },
                                      { u"4"_ustr, css::awt::Key::NUM4 },
                                      { u"5"_ustr, css::awt::Key::NUM5 },
                                      { u"6"_ustr, css::awt::Key::NUM6 },
                                      { u"7"_ustr, css::awt::Key::NUM7 },
                                      { u"8"_ustr, css::awt::Key::NUM8 },
                                      { u"9"_ustr, css::awt::Key::NUM9 },
                                      { u"A"_ustr, css::awt::Key::A },
                                      { u"B"_ustr, css::awt::Key::B },
                                      { u"C"_ustr, css::awt::Key::C },
                                      { u"D"_ustr, css::awt::Key::D },
                                      { u"E"_ustr, css::awt::Key::E },
                                      { u"F"_ustr, css::awt::Key::F },
                                      { u"G"_ustr, css::awt::Key::G },
                                      { u"H"_ustr, css::awt::Key::H },
                                      { u"I"_ustr, css::awt::Key::I },
                                      { u"J"_ustr, css::awt::Key::J },
                                      { u"K"_ustr, css::awt::Key::K },
                                      { u"L"_ustr, css::awt::Key::L },
                                      { u"M"_ustr, css::awt::Key::M },
                                      { u"N"_ustr, css::awt::Key::N },
                                      { u"O"_ustr, css::awt::Key::O },
                                      { u"P"_ustr, css::awt::Key::P },
                                      { u"Q"_ustr, css::awt::Key::Q },
                                      { u"R"_ustr, css::awt::Key::R },
                                      { u"S"_ustr, css::awt::Key::S },
                                      { u"T"_ustr, css::awt::Key::T },
                                      { u"U"_ustr, css::awt::Key::U },
                                      { u"V"_ustr, css::awt::Key::V },
                                      { u"W"_ustr, css::awt::Key::W },
                                      { u"X"_ustr, css::awt::Key::X },
                                      { u"Y"_ustr, css::awt::Key::Y },
                                      { u"Z"_ustr, css::awt::Key::Z },
                                      { u"F1"_ustr, css::awt::Key::F1 },
                                      { u"F2"_ustr, css::awt::Key::F2 },
                                      { u"F3"_ustr, css::awt::Key::F3 },
                                      { u"F4"_ustr, css::awt::Key::F4 },
                                      { u"F5"_ustr, css::awt::Key::F5 },
                                      { u"F6"_ustr, css::awt::Key::F6 },
                                      { u"F7"_ustr, css::awt::Key::F7 },
                                      { u"F8"_ustr, css::awt::Key::F8 },
                                      { u"F9"_ustr, css::awt::Key::F9 },
                                      { u"F10"_ustr, css::awt::Key::F10 },
                                      { u"F11"_ustr, css::awt::Key::F11 },
                                      { u"F12"_ustr, css::awt::Key::F12 },
                                      { u"DOWN"_ustr, css::awt::Key::DOWN },
                                      { u"UP"_ustr, css::awt::Key::UP },
                                      { u"LEFT"_ustr, css::awt::Key::LEFT },
                                      { u"RIGHT"_ustr, css::awt::Key::RIGHT },
                                      { u"HOME"_ustr, css::awt::Key::HOME },
                                      { u"END"_ustr, css::awt::Key::END },
                                      { u"PAGEUP"_ustr, css::awt::Key::PAGEUP },
                                      { u"PAGEDOWN"_ustr, css::awt::Key::PAGEDOWN },
                                      { u"RETURN"_ustr, css::awt::Key::RETURN },
                                      { u"ESCAPE"_ustr, css::awt::Key::ESCAPE },
                                      { u"TAB"_ustr, css::awt::Key::TAB },
                                      { u"BACKSPACE"_ustr, css::awt::Key::BACKSPACE },
                                      { u"SPACE"_ustr, css::awt::Key::SPACE },
                                      { u"INSERT"_ustr, css::awt::Key::INSERT },
                                      { u"DELETE"_ustr, css::awt::Key::DELETE },
                                      { u"ADD"_ustr, css::awt::Key::ADD },
                                      { u"SUBTRACT"_ustr, css::awt::Key::SUBTRACT },
                                      { u"MULTIPLY"_ustr, css::awt::Key::MULTIPLY },
                                      { u"DIVIDE"_ustr, css::awt::Key::DIVIDE },
                                      { u"CUT"_ustr, css::awt::Key::CUT },
                                      { u"COPY"_ustr, css::awt::Key::COPY },
                                      { u"PASTE"_ustr, css::awt::Key::PASTE },
                                      { u"UNDO"_ustr, css::awt::Key::UNDO },
                                      { u"REPEAT"_ustr, css::awt::Key::REPEAT } };

        for (const auto& i : aInfoMap)
        {
            aIdentifierHashMap[i.sIdentifier] = i.nCode;
            aCodeHashMap[i.nCode] = i.sIdentifier;
        }
    }

    sal_Int16 mapIdentifier2Code(const OUString& sIdentifier)
    {
        return aIdentifierHashMap.find(sIdentifier)->second;
    }

    OUString mapCode2Identifier(sal_Int16 nCode) { return aCodeHashMap.find(nCode)->second; }
};
}

class AcceleratorsTest2 : public UnoApiTest
{
    /** the accelerator configuration for testing. */
    Reference<ui::XAcceleratorConfiguration> m_xGlobalAccelCfg;
    /** XCS/XCU based accelerator configuration. */
    Reference<XNameAccess> m_xPrimaryKeys;
    Reference<XNameAccess> m_xSecondaryKeys;

public:
    AcceleratorsTest2()
        : UnoApiTest(u"/framework/qa/cppunit/data"_ustr)
    {
    }

    void setUp() override;
    void tearDown() override;
    Reference<XNameAccess> openConfig(OUString sConfigPath, bool bReadOnly);
    void impl_checkGetKeyCommands(const Reference<XAcceleratorConfiguration>& xAccelCfg,
                                  const Reference<XNameAccess>& xAccess,
                                  const std::vector<OUString>& sKeys);
    void impl_checkSetKeyCommands(const Reference<XAcceleratorConfiguration>& xAccelCfg,
                                  const Reference<XNameAccess>& xPrimaryAccess,
                                  const Reference<XNameAccess>& xSecondaryAccess,
                                  const std::vector<OUString>& sKeys,
                                  const std::vector<OUString>& sCommands);
    void impl_checkRemoveKeyCommands(const Reference<XAcceleratorConfiguration>& xAccelCfg,
                                     const Reference<XNameAccess>& xPrimaryAccess,
                                     const Reference<XNameAccess>& xSecondaryAccess,
                                     const std::vector<OUString>& sKeys);
    void impl_checkGetPreferredKeyEventsForCommandList(
        const Reference<XAcceleratorConfiguration>& xAccelCfg,
        const Reference<XNameAccess>& xPrimaryAccess, const Sequence<OUString>& sCommandList);
    OUString getCommandFromConfiguration(const Reference<XNameAccess>& xAccess,
                                         const OUString& sKey);
    KeyEvent convertShortcut2AWTKey(const OUString& sShortcut);
    void setOfficeLocale(const OUString& sLocale);
    OUString convertAWTKey2Shortcut(const KeyEvent& aKeyEvent);
    OUString getOfficeLocale();
    void SaveDocumentAcceleratorConfiguration(const OUString& sDocCfgName);
    void LoadDocumentAcceleratorConfiguration(const OUString& sDocCfgName);

    void checkGlobalAccelCfg();
    void checkModuleAccelCfg();
    void checkDocumentAccelCfg();

    CPPUNIT_TEST_SUITE(AcceleratorsTest2);
    CPPUNIT_TEST(checkGlobalAccelCfg);
    CPPUNIT_TEST(checkModuleAccelCfg);
    CPPUNIT_TEST(checkDocumentAccelCfg);
    CPPUNIT_TEST_SUITE_END();
};

void AcceleratorsTest2::setUp()
{
    UnoApiTest::setUp();

    Reference<XMultiServiceFactory> xFactory = getMultiServiceFactory();
    m_xGlobalAccelCfg.set(
        xFactory->createInstance(u"com.sun.star.ui.GlobalAcceleratorConfiguration"_ustr),
        UNO_QUERY);
    bool bReadOnly = false;
    Reference<XNameAccess> xConfig2
        = openConfig(u"org.openoffice.Office.Accelerators"_ustr, bReadOnly);
    if (xConfig2)
    {
        m_xPrimaryKeys.set(xConfig2->getByName(u"PrimaryKeys"_ustr), UNO_QUERY);
        m_xSecondaryKeys.set(xConfig2->getByName(u"SecondaryKeys"_ustr), UNO_QUERY);
    }
}

Reference<XNameAccess> AcceleratorsTest2::openConfig(OUString sConfigPath, bool bReadOnly)
{
    Reference<XMultiServiceFactory> xConfigRoot
        = css::configuration::theDefaultProvider::get(comphelper::getProcessComponentContext());

    Sequence<Any> lParams(2);
    lParams.getArray()[0] <<= comphelper::makePropertyValue(u"nodepath"_ustr, sConfigPath);
    lParams.getArray()[1] <<= comphelper::makePropertyValue(u"locale"_ustr, u"*"_ustr);

    Reference<XInterface> aConfig;
    if (bReadOnly)
    {
        aConfig = xConfigRoot->createInstanceWithArguments(
            u"com.sun.star.configuration.ConfigurationAccess"_ustr, lParams);
    }
    else
    {
        aConfig = xConfigRoot->createInstanceWithArguments(
            u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr, lParams);
    }

    Reference<XNameAccess> xConfig(aConfig, UNO_QUERY_THROW);
    return xConfig;
}

void AcceleratorsTest2::tearDown()
{
    m_xGlobalAccelCfg.clear();
    m_xPrimaryKeys.clear();
    m_xSecondaryKeys.clear();
    UnoApiTest::tearDown();
}

void AcceleratorsTest2::checkGlobalAccelCfg()
{
    Reference<XNameAccess> xPrimaryAccess(m_xPrimaryKeys->getByName(u"Global"_ustr),
                                          UNO_QUERY_THROW);
    Reference<XNameAccess> xSecondaryAccess(m_xSecondaryKeys->getByName(u"Global"_ustr),
                                            UNO_QUERY_THROW);

    std::vector<OUString> sKeys;
    sKeys = { u"A_MOD1"_ustr };
    impl_checkGetKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, sKeys);

    sKeys = { u"PASTE"_ustr, u"X_SHIFT"_ustr };
    std::vector<OUString> sCommands{ u".uno:test"_ustr, u".uno:test"_ustr };
    impl_checkSetKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys, sCommands);

    sKeys = { u"C_MOD1"_ustr, u"CUT"_ustr };
    impl_checkRemoveKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys);

    Sequence<OUString> sCommandList{ u".uno:Paste"_ustr, u".uno:CloseWin"_ustr };
    impl_checkGetPreferredKeyEventsForCommandList(m_xGlobalAccelCfg, xPrimaryAccess, sCommandList);
}

void AcceleratorsTest2::checkModuleAccelCfg()
{
    OUString sModules[]{
        u"com.sun.star.frame.StartModule"_ustr,
        u"com.sun.star.drawing.DrawingDocument"_ustr,
        u"com.sun.star.presentation.PresentationDocument"_ustr,
        u"com.sun.star.sheet.SpreadsheetDocument"_ustr,
        u"com.sun.star.text.TextDocument"_ustr,
        // add other modules here
    };

    Reference<XMultiServiceFactory> xFactory = getMultiServiceFactory();
    for (const auto& i : sModules)
    {
        Reference<XAcceleratorConfiguration> xModuleAccelCfg(
            xFactory->createInstanceWithArguments(
                u"com.sun.star.ui.ModuleAcceleratorConfiguration"_ustr, { Any(i) }),
            UNO_QUERY);

        Reference<XNameAccess> xPrimaryModules(m_xPrimaryKeys->getByName(u"Modules"_ustr),
                                               UNO_QUERY);
        Reference<XNameAccess> xSecondaryModules(m_xSecondaryKeys->getByName(u"Modules"_ustr),
                                                 UNO_QUERY);

        std::vector<OUString> sKeys;
        Reference<XNameAccess> xPrimaryAccess(xPrimaryModules->getByName(i), UNO_QUERY);
        Reference<XNameAccess> xSecondaryAccess(xSecondaryModules->getByName(i), UNO_QUERY);

        if (i == "com.sun.star.presentation.PresentationDocument")
        {
            sKeys = { u"A_SHIFT_MOD1_MOD2"_ustr };
        }
        else if (i == "com.sun.star.sheet.SpreadsheetDocument")
        {
            sKeys = { u"B_MOD1"_ustr };
        }
        else if (i == "com.sun.star.text.TextDocument")
        {
            sKeys = { u"F11_MOD1"_ustr };
        }
        else
        {
            sKeys = { u"A_MOD1"_ustr };
        }
        impl_checkGetKeyCommands(xModuleAccelCfg, xPrimaryAccess, sKeys);

        std::vector<OUString> sCommands;
        if (i == "com.sun.star.presentation.PresentationDocument")
        {
            sKeys = { u"A_SHIFT_MOD1_MOD2"_ustr };
            sCommands = { u".uno:test"_ustr };
        }
        else if (i == "com.sun.star.sheet.SpreadsheetDocument")
        {
            sKeys = { u"B_MOD1"_ustr };
            sCommands = { u".uno:test"_ustr };
        }
        else if (i == "com.sun.star.text.TextDocument")
        {
            sKeys = { u"F11_MOD1"_ustr };
            sCommands = { u".uno:test"_ustr };
        }
        else
        {
            sKeys = { u"PASTE"_ustr };
            sCommands = { u".uno:test"_ustr };
        }
        impl_checkSetKeyCommands(xModuleAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys,
                                 sCommands);

        if (i == "com.sun.star.presentation.PresentationDocument")
        {
            sKeys = { u"A_SHIFT_MOD1_MOD2"_ustr };
        }
        else if (i == "com.sun.star.sheet.SpreadsheetDocument")
        {
            sKeys = { u"F5_SHIFT_MOD1"_ustr };
        }
        else if (i == "com.sun.star.text.TextDocument")
        {
            sKeys = { u"BACKSPACE_MOD2"_ustr };
        }
        else
        {
            sKeys = { u"C_MOD1"_ustr };
        }
        impl_checkRemoveKeyCommands(xModuleAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys);

        Sequence<OUString> sCommandList;
        if (i == "com.sun.star.presentation.PresentationDocument")
        {
            sCommandList = { u".uno:Presentation"_ustr };
        }
        else if (i == "com.sun.star.sheet.SpreadsheetDocument")
        {
            sCommandList = { u".uno:InsertCell"_ustr };
        }
        else if (i == "com.sun.star.text.TextDocument")
        {
            sCommandList = { u".uno:SelectionModeBlock"_ustr };
        }
        else
        {
            sCommandList = { u".uno:Cut"_ustr };
        }
        impl_checkGetPreferredKeyEventsForCommandList(xModuleAccelCfg, xPrimaryAccess,
                                                      sCommandList);
    }
}

static OUString getOfficeSettingsValue(const Reference<XMultiServiceFactory>& msf,
                                       const OUString& setting)
{
    Reference<XPropertySet> pthSettings(
        msf->createInstance(u"com.sun.star.comp.framework.PathSettings"_ustr), UNO_QUERY);
    return pthSettings->getPropertyValue(setting).get<OUString>();
}

/**
 * This method gets the user dir of the connected office
 */
static OUString getOfficeUserPath(const Reference<XMultiServiceFactory>& msf)
{
    // get a folder located in the user dir
    OUString userPath = getOfficeSettingsValue(msf, u"UserConfig"_ustr);

    // strip the returned folder to the user dir
    if (userPath.endsWith("/"))
    {
        userPath = userPath.copy(0, userPath.getLength() - 1);
    }
    int index = userPath.lastIndexOf('/');
    if (index != -1)
    {
        userPath = userPath.copy(0, index);
    }

    return userPath;
}

/*
 * Concat a _sRelativePathToAdd to a _sPath and append a '/' to the _sPath only if need.
 *
 * @return a right concatenated path
 */
static OUString appendPath(const OUString& _sPath, std::u16string_view _sRelativePathToAdd)
{
    OUString sNewPath = _sPath;
    OUString fs(static_cast<sal_Unicode>(std::filesystem::path::preferred_separator));
    if (_sPath.startsWith("file:"))
    {
        fs = u"/"_ustr; // we use a file URL so only '/' is allowed.
    }
    if (!(sNewPath.endsWith("/") || sNewPath.endsWith("\\")))
    {
        sNewPath += fs;
    }
    sNewPath += _sRelativePathToAdd;
    return sNewPath;
}

void AcceleratorsTest2::checkDocumentAccelCfg()
{
    Reference<XMultiServiceFactory> msf = getMultiServiceFactory();
    OUString tempDirURL = getOfficeUserPath(msf) + "/test-temp/";
    std::filesystem::create_directories(tempDirURL.toUtf8().getStr());
    OUString sDocCfgName = appendPath(tempDirURL, u"test.cfg"_ustr);
    SaveDocumentAcceleratorConfiguration(sDocCfgName);
    LoadDocumentAcceleratorConfiguration(sDocCfgName);
}

void AcceleratorsTest2::impl_checkGetKeyCommands(
    const Reference<XAcceleratorConfiguration>& xAccelCfg, const Reference<XNameAccess>& xAccess,
    const std::vector<OUString>& sKeys)
{
    for (size_t i = 0; i < sKeys.size(); ++i)
    {
        if (xAccess->hasByName(sKeys[i])
            && getCommandFromConfiguration(xAccess, sKeys[i]).getLength() > 0)
        {
            OUString sCmdFromCache; // get a value using XAcceleratorConfiguration API
            OUString sCmdFromConfiguration; // get a value using configuration API

            // GET shortcuts/commands using XAcceleratorConfiguration API
            sCmdFromCache = xAccelCfg->getCommandByKeyEvent(convertShortcut2AWTKey(sKeys[i]));

            // GET shortcuts/commands using configuration API
            sCmdFromConfiguration = getCommandFromConfiguration(xAccess, sKeys[i]);

            CPPUNIT_ASSERT_MESSAGE(
                "values are different by XAcceleratorConfiguration API and configuration API!",
                sCmdFromCache.equals(sCmdFromConfiguration));

            OUString sLocale = u"es"_ustr;
            setOfficeLocale(sLocale);
            sCmdFromConfiguration = getCommandFromConfiguration(xAccess, sKeys[i]);

            sLocale = u"en-US"_ustr;
            setOfficeLocale(sLocale); //reset to default locale
        }
        else
        {
            // doesn't exist
        }
    }
}

void AcceleratorsTest2::impl_checkSetKeyCommands(
    const Reference<XAcceleratorConfiguration>& xAccelCfg,
    const Reference<XNameAccess>& xPrimaryAccess, const Reference<XNameAccess>& xSecondaryAccess,
    const std::vector<OUString>& sKeys, const std::vector<OUString>& sCommands)
{
    for (size_t i = 0; i < sKeys.size(); ++i)
    {
        if (!xPrimaryAccess->hasByName(sKeys[i]) && !xSecondaryAccess->hasByName(sKeys[i]))
        {
            xAccelCfg->setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
            xAccelCfg->store();
            if (xPrimaryAccess->hasByName(sKeys[i]))
            {
                //                System.out.println("add " + sKeys[i] + " successfully!");
            }
            else
            {
                //                System.out.println("add " + sKeys[i] + " failed!");
            }
        }
        else if (xPrimaryAccess->hasByName(sKeys[i]))
        {
            OUString sOriginalCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
            if (!sCommands[i].equals(sOriginalCommand))
            {
                xAccelCfg->setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
                xAccelCfg->store();

                OUString sChangedCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
                if (sCommands[i].equals(sChangedCommand))
                {
                    //                    System.out.println("change " + sKeys[i] + " successfully!");
                }
                else
                {
                    //                    System.out.println("change " + sKeys[i] + " failed!");
                }
            }
            else
            {
                //                System.out.println(sKeys[i] + " already exist!");
            }
        }
        else if (xSecondaryAccess->hasByName(sKeys[i]))
        {
            OUString sOriginalCommand = getCommandFromConfiguration(xSecondaryAccess, sKeys[i]);
            if (!sCommands[i].equals(sOriginalCommand))
            {
                xAccelCfg->setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
                xAccelCfg->store();

                OUString sChangedCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
                if (sCommands[i].equals(sChangedCommand))
                {
                    //                    System.out.println("change " + sKeys[i] + " successfully!");
                }
                else
                {
                    //                    System.out.println("change " + sKeys[i] + " failed!");
                }
            }
            else
            {
                //                System.out.println(sKeys[i] + " already exist!");
            }
        }
    }
}

void AcceleratorsTest2::impl_checkRemoveKeyCommands(
    const Reference<XAcceleratorConfiguration>& xAccelCfg,
    const Reference<XNameAccess>& xPrimaryAccess, const Reference<XNameAccess>& xSecondaryAccess,
    const std::vector<OUString>& sKeys)
{
    for (size_t i = 0; i < sKeys.size(); i++)
    {
        if (!xPrimaryAccess->hasByName(sKeys[i]) && !xSecondaryAccess->hasByName(sKeys[i]))
        {
            //            System.out.println(sKeys[i] + " doesn't exist!");
        }
        else if (xPrimaryAccess->hasByName(sKeys[i]))
        {
            xAccelCfg->removeKeyEvent(convertShortcut2AWTKey(sKeys[i]));
            xAccelCfg->store();
            if (!xPrimaryAccess->hasByName(sKeys[i]))
            {
                //                System.out.println("Remove " + sKeys[i] + " successfully!");
            }
            else
            {
                //                System.out.println("Remove " + sKeys[i] + " failed!");
            }
        }
        else if (xSecondaryAccess->hasByName(sKeys[i]))
        {
            xAccelCfg->removeKeyEvent(convertShortcut2AWTKey(sKeys[i]));
            xAccelCfg->store();
            if (!xSecondaryAccess->hasByName(sKeys[i]))
            {
                //                System.out.println("Remove " + sKeys[i] + " successfully!");
            }
            else
            {
                //                System.out.println("Remove " + sKeys[i] + " failed!");
            }
        }
    }
}

void AcceleratorsTest2::impl_checkGetPreferredKeyEventsForCommandList(
    const Reference<XAcceleratorConfiguration>& xAccelCfg,
    const Reference<XNameAccess>& xPrimaryAccess, const Sequence<OUString>& sCommandList)
{
    Sequence<Any> oKeyEvents = xAccelCfg->getPreferredKeyEventsForCommandList(sCommandList);
    for (int i = 0; i < oKeyEvents.getLength(); i++)
    {
        KeyEvent aKeyEvent;
        oKeyEvents[i] >>= aKeyEvent;
        OUString sKeyEvent = convertAWTKey2Shortcut(aKeyEvent);

        OUString sCmdFromConfiguration = getCommandFromConfiguration(xPrimaryAccess, sKeyEvent);
        if (sCommandList[i].equals(sCmdFromConfiguration))
        {
            //            System.out.println("get preferred key correctly!");
        }
        else
        {
            //            System.out.println("get preferred key failed!");
        }
    }
}

OUString AcceleratorsTest2::getCommandFromConfiguration(const Reference<XNameAccess>& xAccess,
                                                        const OUString& sKey)
{
    OUString sCommand;

    if (xAccess->hasByName(sKey))
    {
        Reference<XNameAccess> xKey(xAccess->getByName(sKey), UNO_QUERY);
        Reference<XNameAccess> xCommand(xKey->getByName(u"Command"_ustr), UNO_QUERY);

        OUString sLocale = getOfficeLocale();
        if (xCommand->hasByName(sLocale))
        {
            xCommand->getByName(sLocale) >>= sCommand;
        }
    }

    return sCommand;
}

void AcceleratorsTest2::LoadDocumentAcceleratorConfiguration(const OUString& sDocCfgName)
{
    Reference<XSingleServiceFactory> xStorageFactory(
        getMultiServiceFactory()->createInstance(u"com.sun.star.embed.StorageFactory"_ustr), UNO_QUERY);

    Sequence<Any> aArgs{ Any(sDocCfgName), Any(ElementModes::READ) };
    Reference<XStorage> xRootStorage(xStorageFactory->createInstanceWithArguments(aArgs),
                                     UNO_QUERY);

    Reference<XStorage> xUIConfig
        = xRootStorage->openStorageElement(u"Configurations2"_ustr, ElementModes::READ);

    Reference<XAcceleratorConfiguration> xDocumentAccelCfg(
        getMultiServiceFactory()->createInstanceWithArguments(
            u"com.sun.star.ui.DocumentAcceleratorConfiguration"_ustr, { Any(xUIConfig) }),
        UNO_QUERY);

    // TODO: throws css::container::NoSuchElementException
    try
    {
        xDocumentAccelCfg->getCommandByKeyEvent(convertShortcut2AWTKey(u"F2"_ustr));
    }
    catch (NoSuchElementException)
    {
    }
}

void AcceleratorsTest2::SaveDocumentAcceleratorConfiguration(const OUString& sDocCfgName)
{
    Reference<XSingleServiceFactory> xStorageFactory(
        getMultiServiceFactory()->createInstance(u"com.sun.star.embed.StorageFactory"_ustr), UNO_QUERY);

    Sequence<Any> aArgs{ Any(sDocCfgName), Any(ElementModes::WRITE) };
    Reference<XStorage> xRootStorage(xStorageFactory->createInstanceWithArguments(aArgs),
                                     UNO_QUERY);

    Reference<XStorage> xUIConfig
        = xRootStorage->openStorageElement(u"Configurations2"_ustr, ElementModes::WRITE);

    Reference<XUIConfigurationManager> xCfgMgr(
        getMultiServiceFactory()->createInstance(u"com.sun.star.ui.UIConfigurationManager"_ustr),
        UNO_QUERY);

    Reference<XUIConfigurationStorage> xUICfgStore(xCfgMgr, UNO_QUERY);
    xUICfgStore->setStorage(xUIConfig);

    Reference<XPropertySet> xUIConfigProps(xUIConfig, UNO_QUERY);
    xUIConfigProps->setPropertyValue(u"MediaType"_ustr,
                                     Any(u"application/vnd.sun.xml.ui.configuration"_ustr));

    if (xCfgMgr)
    {
        Reference<XAcceleratorConfiguration> xTargetAccMgr(xCfgMgr->getShortCutManager());
        Reference<XUIConfigurationPersistence> xCommit1(xTargetAccMgr);
        Reference<XUIConfigurationPersistence> xCommit2(xCfgMgr, UNO_QUERY);
        xCommit1->store();
        xCommit2->store();

        Reference<XTransactedObject> xCommit3(xRootStorage, UNO_QUERY);
        xCommit3->commit();
    }
}

// [-loplugin:stringviewparam]
KeyEvent AcceleratorsTest2::convertShortcut2AWTKey(const OUString& sShortcut)
{
    KeyEvent aKeyEvent;
    KeyMapping aKeyMapping;

    sal_Int32 nIndex = 0;
    OUString id = sShortcut.getToken(0, '_', nIndex);
    aKeyEvent.KeyCode = aKeyMapping.mapIdentifier2Code(id);
    while (nIndex >= 0)
    {
        OUString aToken = sShortcut.getToken(0, '_', nIndex);
        if (aToken.equals(u"SHIFT"_ustr))
        {
            aKeyEvent.Modifiers |= KeyModifier::SHIFT;
        }
        else if (aToken.equals(u"MOD1"_ustr))
        {
            aKeyEvent.Modifiers |= KeyModifier::MOD1;
        }
        else if (aToken.equals(u"MOD2"_ustr))
        {
            aKeyEvent.Modifiers |= KeyModifier::MOD2;
        }
    }

    return aKeyEvent;
}

void AcceleratorsTest2::setOfficeLocale(const OUString& sLocale)
{
    Reference<XNameAccess> xRootConfig
        = openConfig(u"org.openoffice.Setup"_ustr, /*bReadOnly*/ false);
    if (xRootConfig)
    {
        Reference<XNameAccess> xLocale(xRootConfig->getByName(u"L10N"_ustr), UNO_QUERY);
        Reference<XPropertySet> xSet(xLocale, UNO_QUERY);
        xSet->setPropertyValue(u"ooLocale"_ustr, Any(sLocale));
        Reference<XChangesBatch> xBatch(xRootConfig, UNO_QUERY);
        xBatch->commitChanges();
    }
}

OUString AcceleratorsTest2::convertAWTKey2Shortcut(const KeyEvent& aKeyEvent)
{
    KeyMapping aKeyMapping;
    OUString sShortcut = aKeyMapping.mapCode2Identifier(aKeyEvent.KeyCode);

    if ((aKeyEvent.Modifiers & KeyModifier::SHIFT) == KeyModifier::SHIFT)
    {
        sShortcut += "_SHIFT";
    }
    if ((aKeyEvent.Modifiers & KeyModifier::MOD1) == KeyModifier::MOD1)
    {
        sShortcut += "_MOD1";
    }
    if ((aKeyEvent.Modifiers & KeyModifier::MOD2) == KeyModifier::MOD2)
    {
        sShortcut += "_MOD2";
    }

    return sShortcut;
}

OUString AcceleratorsTest2::getOfficeLocale()
{
    OUString sLocale;

    Reference<XNameAccess> xRootConfig
        = openConfig(u"org.openoffice.Setup"_ustr, /*bReadOnly*/ true);

    if (xRootConfig)
    {
        Reference<XNameAccess> xLocale(xRootConfig->getByName(u"L10N"_ustr), UNO_QUERY);
        Reference<XPropertySet> xSet(xLocale, UNO_QUERY);
        sLocale = xSet->getPropertyValue(u"ooLocale"_ustr).get<OUString>();
    }

    return sLocale;
}

CPPUNIT_TEST_SUITE_REGISTRATION(AcceleratorsTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
