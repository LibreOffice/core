/*************************************************************************
 *
 *  $RCSfile: pathsettings.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ______________________________________________
// my own includes

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

#ifndef __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_
#include <services/pathsettings.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

// ______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

// ______________________________________________
// includes of other projects

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

// ______________________________________________
//  non exported const

#define CFG_READONLY_DEFAULT    sal_False

// ______________________________________________
//  namespace

namespace framework
{

const ::rtl::OUString PathSettingsCfg::PropNames[] =
{
    PATHSETTINGS_PROPNAME_ADDIN         ,
    PATHSETTINGS_PROPNAME_AUTOCORRECT   ,
    PATHSETTINGS_PROPNAME_AUTOTEXT      ,
    PATHSETTINGS_PROPNAME_BACKUP        ,
    PATHSETTINGS_PROPNAME_BASIC         ,
    PATHSETTINGS_PROPNAME_BITMAP        ,
    PATHSETTINGS_PROPNAME_CONFIG        ,
    PATHSETTINGS_PROPNAME_DICTIONARY    ,
    PATHSETTINGS_PROPNAME_FAVORITE      ,
    PATHSETTINGS_PROPNAME_FILTER        ,
    PATHSETTINGS_PROPNAME_GALLERY       ,
    PATHSETTINGS_PROPNAME_GRAPHIC       ,
    PATHSETTINGS_PROPNAME_HELP          ,
    PATHSETTINGS_PROPNAME_LINGUISTIC    ,
    PATHSETTINGS_PROPNAME_MODULE        ,
    PATHSETTINGS_PROPNAME_PALETTE       ,
    PATHSETTINGS_PROPNAME_PLUGIN        ,
    PATHSETTINGS_PROPNAME_STORAGE       ,
    PATHSETTINGS_PROPNAME_TEMP          ,
    PATHSETTINGS_PROPNAME_TEMPLATE      ,
    PATHSETTINGS_PROPNAME_UICONFIG      ,
    PATHSETTINGS_PROPNAME_USERCONFIG    ,
    PATHSETTINGS_PROPNAME_USERDICTIONARY,
    PATHSETTINGS_PROPNAME_WORK
};

const css::beans::Property PathSettingsCfg::Properties[] =
{
    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_ADDIN],
        PathSettingsCfg::E_ADDIN,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_AUTOCORRECT],
        PathSettingsCfg::E_AUTOCORRECT,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_AUTOTEXT],
        PathSettingsCfg::E_AUTOTEXT,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_BACKUP],
        PathSettingsCfg::E_BACKUP,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_BASIC],
        PathSettingsCfg::E_BASIC,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_BITMAP],
        PathSettingsCfg::E_BITMAP,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_CONFIG],
        PathSettingsCfg::E_CONFIG,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_DICTIONARY],
        PathSettingsCfg::E_DICTIONARY,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_FAVORITE],
        PathSettingsCfg::E_FAVORITE,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_FILTER],
        PathSettingsCfg::E_FILTER,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_GALLERY],
        PathSettingsCfg::E_GALLERY,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_GRAPHIC],
        PathSettingsCfg::E_GRAPHIC,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_HELP],
        PathSettingsCfg::E_HELP,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_LINGUISTIC],
        PathSettingsCfg::E_LINGUISTIC,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_MODULE],
        PathSettingsCfg::E_MODULE,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_PALETTE],
        PathSettingsCfg::E_PALETTE,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_PLUGIN],
        PathSettingsCfg::E_PLUGIN,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_STORAGE],
        PathSettingsCfg::E_STORAGE,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_TEMP],
        PathSettingsCfg::E_TEMP,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_TEMPLATE],
        PathSettingsCfg::E_TEMPLATE,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_UICONFIG],
        PathSettingsCfg::E_UICONFIG,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_USERCONFIG],
        PathSettingsCfg::E_USERCONFIG,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_USERDICTIONARY],
        PathSettingsCfg::E_USERDICTIONARY,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND),

    css::beans::Property(
        PathSettingsCfg::PropNames[PathSettingsCfg::E_WORK],
        PathSettingsCfg::E_WORK,
        ::getCppuType((rtl::OUString*)NULL),
        css::beans::PropertyAttribute::BOUND)
};

// ______________________________________________

/** it fill this new cache instance and prepare all neccessary structures.
    Note: It's very important for our work, that another helper service PathSubstitution could be created here.
    It's needed later .. and without its work the pathes which are provided here will not valid.
    That's why we throw a RuntimeException in case this service couldn't be created!
 */

PathSettingsCfg::PathSettingsCfg( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    :   ::utl::ConfigItem( DECLARE_ASCII("Office.Common/Path/Current") )
{
    // create the needed substitution service.
    // We must replace all used variables inside readed path values.
    // In case we can't do so ... the whole office can't work realy.
    // That's why it seams to be OK to throw a RuntimeException then.
    if (xSMGR.is())
    {
        m_xSubstitution = css::uno::Reference< css::util::XStringSubstitution >(
                            xSMGR->createInstance(SERVICENAME_SUBSTITUTEPATHVARIABLES),
                            css::uno::UNO_QUERY);
    }
    if (!m_xSubstitution.is())
        throw css::uno::RuntimeException(DECLARE_ASCII("Could not create substitution service. Path settings will not work."),css::uno::Reference< css::uno::XInterface >());

    // create mapping for path names to her ID's
    // This hash content is fix and already defined by some fix arrays of the class PathSettingsCfg.
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_ADDIN         ]] = PathSettingsCfg::E_ADDIN         ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_AUTOCORRECT   ]] = PathSettingsCfg::E_AUTOCORRECT   ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_AUTOTEXT      ]] = PathSettingsCfg::E_AUTOTEXT      ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_BACKUP        ]] = PathSettingsCfg::E_BACKUP        ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_BASIC         ]] = PathSettingsCfg::E_BASIC         ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_BITMAP        ]] = PathSettingsCfg::E_BITMAP        ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_CONFIG        ]] = PathSettingsCfg::E_CONFIG        ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_DICTIONARY    ]] = PathSettingsCfg::E_DICTIONARY    ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_FAVORITE      ]] = PathSettingsCfg::E_FAVORITE      ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_FILTER        ]] = PathSettingsCfg::E_FILTER        ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_GALLERY       ]] = PathSettingsCfg::E_GALLERY       ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_GRAPHIC       ]] = PathSettingsCfg::E_GRAPHIC       ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_HELP          ]] = PathSettingsCfg::E_HELP          ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_LINGUISTIC    ]] = PathSettingsCfg::E_LINGUISTIC    ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_MODULE        ]] = PathSettingsCfg::E_MODULE        ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_PALETTE       ]] = PathSettingsCfg::E_PALETTE       ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_PLUGIN        ]] = PathSettingsCfg::E_PLUGIN        ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_STORAGE       ]] = PathSettingsCfg::E_STORAGE       ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_TEMP          ]] = PathSettingsCfg::E_TEMP          ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_TEMPLATE      ]] = PathSettingsCfg::E_TEMPLATE      ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_UICONFIG      ]] = PathSettingsCfg::E_UICONFIG      ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_USERCONFIG    ]] = PathSettingsCfg::E_USERCONFIG    ;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_USERDICTIONARY]] = PathSettingsCfg::E_USERDICTIONARY;
    m_lIDMap[PathSettingsCfg::PropNames[PathSettingsCfg::E_WORK          ]] = PathSettingsCfg::E_WORK          ;

    // use fix list of all needed path names to read her values from the configuration.
    // Call impl_read() method mit optimization parameter "bSearchID=sal_False"!
    // So hash map isn't used to map from given property name to the corresponding ID.
    // It's not neccessary here. Because we have a fix list of names, which correspond
    // directly to an ID by it's position inside the array ...
    const css::uno::Sequence< ::rtl::OUString > lNames(PathSettingsCfg::PropNames,PATHSETTINGS_PROPCOUNT);
    impl_read(lNames,sal_False);
}

// ______________________________________________

/** nothing to do here ...
    Because we are a write-through cache ... all items should be already saved.
    But may it's better to check it for debug purposes.
 */

PathSettingsCfg::~PathSettingsCfg()
{
    LOG_ASSERT(!IsModified(), "PathSettingsCfg::~PathSettingsCfg()\nModified state of a write-through cache shouldn't be possible at the end of lifetime.\n")
}

// ______________________________________________

/** There are some outside changed items, which should be updated inside this cache too.
 */

void PathSettingsCfg::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& lPropertyNames )
{
    // Attention: These list of pathes isn't well known nor fix. We can't use our optimization here,
    // that the prop ID/Handle is the same as the current array position.
    // That's why we must use our special hash for mapping path names to her corresponding ID.
    // => second parameter bSearchID=sal_True must be used here
    impl_read(lPropertyNames,sal_True);
}

// ______________________________________________

/** return the current path value for this entry.
 */

::rtl::OUString PathSettingsCfg::getPath( EPropHandle nID ) const
{
    // SAFE {
    ReadGuard aReadLock(m_aLock);
    return m_lPathes[nID].sValue;
}

// ______________________________________________

/** set he new path value for this entry.
    But it's done only, if it will change realy.
    And because this class is implemented as a write through cache,
    this new value is written immediatly to the configuration.
 */

void PathSettingsCfg::setPath(       EPropHandle      nID    ,
                               const ::rtl::OUString& sValue )
{
    // SAFE {
    WriteGuard aWriteLock(m_aLock);

    // has somthing changed?
    if (m_lPathes[nID].sValue.equals(sValue))
        return;

    // is it readonly?
    // It's an implementation error if this method is called for readonly properties!
    // Our helper class OPropertySetHelper throws normaly a PropertyVetoExceptio, if somehwere
    // tries to call setPropertyValue() for readonly properties.
    if (m_lPathes[nID].bReadOnly)
    {
        LOG_WARNING("PathSettingsCfg::setPath()", "Unexpected set call for readonly path detected!")
        return;
    }

    // take over the new value into our cache structures
    m_lPathes[nID].sValue = sValue;

    aWriteLock.unlock();
    // } SAFE

    // update the configuration immediatly
    sal_Bool bMultiPath = isMultiPath(nID);

    css::uno::Sequence< ::rtl::OUString > lNames  (1);
    css::uno::Sequence< css::uno::Any >   lValues (1);
    ::rtl::OUString*                      pNames  = lNames.getArray();
    css::uno::Any*                        pValues = lValues.getArray();
    ::rtl::OUString                       sReSubst;

    // SAFE {
    ReadGuard aReadLock(m_aLock);
    if (bMultiPath)
    {
        css::uno::Sequence< ::rtl::OUString > lMulti;
        sal_Int32 nToken = 0;
        do
        {
            ::rtl::OUString sToken = m_lPathes[nID].sValue.getToken(0, ';', nToken);
            if (sToken.getLength())
            {
                sReSubst = m_xSubstitution->reSubstituteVariables(sToken);
                lMulti.realloc(lMulti.getLength()+1);
                lMulti[lMulti.getLength()-1] = sReSubst;
            }
        }
        while(nToken>=0);
        pNames[0]    = m_lPathes[nID].sPath;
        pValues[0] <<= lMulti;
    }
    else
    {
        pNames[0]    = m_lPathes[nID].sPath;
        sReSubst     = m_xSubstitution->reSubstituteVariables(m_lPathes[nID].sValue);
        pValues[0] <<= sReSubst;
    }
    aReadLock.unlock();
    // } SAFE

    PutProperties(lNames, lValues);
}

// ______________________________________________

/** return the readonly state of a path entry.
 */

sal_Bool PathSettingsCfg::isReadOnly( EPropHandle nID ) const
{
    // SAFE {
    ReadGuard aReadLock(m_aLock);
    return m_lPathes[nID].bReadOnly;
}

// ______________________________________________

/** return information about the path type of a path entry.
    We know single and multi paths.
 */

sal_Bool PathSettingsCfg::isMultiPath( EPropHandle nID ) const
{
    // multi paths are fix be definition!
    sal_Bool bKnownMulti = (nID == PathSettingsCfg::E_AUTOCORRECT||
                            nID == PathSettingsCfg::E_AUTOTEXT   ||
                            nID == PathSettingsCfg::E_BASIC      ||
                            nID == PathSettingsCfg::E_GALLERY    ||
                            nID == PathSettingsCfg::E_PLUGIN     ||
                            nID == PathSettingsCfg::E_TEMPLATE   ||
                            nID == PathSettingsCfg::E_UICONFIG   );
    #ifdef ENABLE_ASSERTIONS
        // But we detected it during reading from the configuration too.
        // We should use this information to check if something was changed for this definition.
        // May our method returns the wrong state then!
        // SAFE {
        ReadGuard aReadLock(m_aLock);
        LOG_ASSERT(bKnownMulti==m_lPathes[nID].bMultiPath, "PathSettingsCfg::isMultiPath()\nThere seams to be a difference between known and detected multi pathes!\n")
        aReadLock.unlock();
        // } SAFE
    #endif
    return bKnownMulti;
}

// ______________________________________________

/** return a descriptor for all supported properties.
    Normaly this descriptor is fix. So you can use the static const value "Properties".
    But the readonly state of a property must be detected during runtime.
    So we must path this structure on demand.
 */

const css::uno::Sequence< css::beans::Property > PathSettingsCfg::getPropertyDescriptor() const
{
    sal_Int32                                  nCount = PATHSETTINGS_PROPCOUNT;
    css::uno::Sequence< css::beans::Property > lDesc  (Properties,nCount);
    css::beans::Property*                      pDesc  = lDesc.getArray();

    // SAFE {
    ReadGuard aReadLock(m_aLock);
    for (sal_Int32 p=0; p<nCount; ++p)
    {
        if (m_lPathes[p].bReadOnly)
            pDesc[p].Attributes |= css::beans::PropertyAttribute::READONLY;
        else
            pDesc[p].Attributes &= ~css::beans::PropertyAttribute::READONLY;
    }
    aReadLock.unlock();
    // } SAFE
    return lDesc;
}

// ______________________________________________

/** return the corresponding property handle for the given property name.
    We use an internal fix hash map to do so.
    Attention: If you call this method e.g. with an unknown property name
    the returned ID will be undefined! Please check the returned boolean
    state of this method everytimes. Otherwhise we try to let it crash then :-)
 */

sal_Bool PathSettingsCfg::mapName2Handle( const ::rtl::OUString& sName ,
                                                EPropHandle&     rID   ) const
{
    // SAFE {
    ReadGuard aReadLock(m_aLock);
    NameToHandleHash::const_iterator pIt      = m_lIDMap.find(sName);
    sal_Bool                         bSuccess = (pIt!=m_lIDMap.end());
    aReadLock.unlock();
    // } SAFE

    if (bSuccess)
        rID = (EPropHandle)(pIt->second);
    else
        // let it crash :-) Better to find wrong using of this method!
        rID = (EPropHandle)-1;
    return bSuccess;
}

// ______________________________________________

/** read the path values from the configuration.
    The special parameter bSearchID enable/disable optional mapping of
    property names to her property handle. As optimization it can be disabled
    if the caller is hure, that the array index inside the parameter lNames
    can be used as such handle without any check. That can be true for
    sorted, fix and full filled lists of properties only! See struct PropNames/EPropHandle too!
 */

void PathSettingsCfg::impl_read( const css::uno::Sequence< ::rtl::OUString >& lNames    ,
                                       sal_Bool                               bSearchID )
{
    sal_Int32                                   nCount    = lNames.getLength();
    const css::uno::Sequence< css::uno::Any >   lValues   = ConfigItem::GetProperties(lNames);
    const css::uno::Sequence< sal_Bool >        lROStates = ConfigItem::GetReadOnlyStates(lNames);
    const ::rtl::OUString*                      pNames    = lNames.getConstArray();
    const css::uno::Any*                        pValues   = lValues.getConstArray();
    const sal_Bool*                             pROStates = lROStates.getConstArray();

    // All getted list (names, values, readonly states) must work together.
    // They must have the same size. Otherwhise combination of it can produce wrong results!
    if ( lValues.getLength()!=lNames.getLength() || lROStates.getLength()!=lNames.getLength() )
    {
        LOG_WARNING("PathSettingsCfg::impl_read()", "GetProperties() or GetReadOnlyStates does not return valid count of items.")
        return;
    }

    for (sal_Int32 n=0; n<nCount; ++n)
    {
        // Dont define these variables outside of this loop scope!
        // Otherwhise you have to be shure that they will be reseted for every new loop ...
        ::rtl::OUString sTempVal;
        ::rtl::OUString sPathVal;
        sal_Bool        bMulti  = sal_False;

        if (!pValues[n].hasValue())
        {
            LOG_WARNING("PathSettingsCfg::impl_read()", "Missing a path value. Item will be ignored.")
            continue;
        }

        // get the path value
        switch (pValues[n].getValueTypeClass())
        {
            // single pathes
            case ::com::sun::star::uno::TypeClass_STRING :
            {
                if (!(pValues[n]>>=sTempVal))
                {
                    LOG_WARNING("PathSettingsCfg::impl_read()", "Could not unpack path value.")
                    continue;
                }

                sPathVal = m_xSubstitution->substituteVariables(sTempVal,sal_False);
                bMulti   = sal_False;
                break;
            }

            // multi pathes
            case ::com::sun::star::uno::TypeClass_SEQUENCE :
            {
                css::uno::Sequence< ::rtl::OUString > lMulti;
                if (!(pValues[n]>>=lMulti))
                {
                    LOG_WARNING("PathSettingsCfg::impl_read()", "Could not unpack multi path value.")
                    continue;
                }

                ::rtl::OUStringBuffer  sBuffer     (256);
                const ::rtl::OUString* pMulti      = lMulti.getConstArray();
                sal_Int32              nMultiCount = lMulti.getLength();
                for (sal_Int32 m=0; m<nMultiCount; ++m)
                {
                    sTempVal = m_xSubstitution->substituteVariables(pMulti[m],sal_False);
                    sBuffer.append(sTempVal);
                    if (m<nMultiCount-1)
                        sBuffer.appendAscii(";");
                }
                sPathVal = sBuffer.makeStringAndClear();
                bMulti   = sal_True;
                break;
            }

            // unknown!
            default:
            {
                LOG_WARNING("PathSettingsCfg::impl_read()", "Unknown path type detected!")
                continue;
            }
        }

        // insert new value inside internal structures
        EPropHandle nID = (EPropHandle)n;
        if (bSearchID)
        {
            if (!mapName2Handle(pNames[n],nID))
            {
                LOG_WARNING("PathSettingsCfg::impl_read()", "Mapping from name to ID failed!")
                continue;
            }
        }

        m_lPathes[nID].sPath      = pNames[n];
        m_lPathes[nID].sValue     = sPathVal;
        m_lPathes[nID].bReadOnly  = pROStates[n];
        m_lPathes[nID].bMultiPath = bMulti;
    }
}

// ______________________________________________

/** check if the given path value seams to be a valid URL or system path.
 */

sal_Bool PathSettingsCfg::isValidValue( const ::rtl::OUString& sValue     ,
                                              sal_Bool         bMultiPath ) const
{
    sal_Bool bOk = sal_True;

    if (bMultiPath)
    {
        sal_Int32 nToken = 0;
        do
        {
            ::rtl::OUString sToken = sValue.getToken(0, ';', nToken);
            if (sToken.getLength())
                bOk = !INetURLObject(sToken).HasError();
        }
        while(nToken>=0 && bOk);
    }
    else
    {
        bOk = !INetURLObject(sValue).HasError();
    }

    return bOk;
}

// ______________________________________________

/** it checks the given path value and tries to correct it.
    Correction can be done by substitution of variables.
    May the user tried to set a value which includes such variables.
 */

sal_Bool PathSettingsCfg::checkAndSubstituteValue( ::rtl::OUString& sValue     ,
                                                   sal_Bool         bMultiPath ) const
{
    sal_Bool bOk = isValidValue(sValue,bMultiPath);

    if (!bOk)
    {
        try
        {
            // SAFE {
            ReadGuard aReadLock(m_aLock);
            ::rtl::OUString sSubst = m_xSubstitution->substituteVariables(sValue,sal_True);
            aReadLock.unlock();
            // } SAFE
            bOk = PathSettingsCfg::isValidValue(sSubst,bMultiPath);
            if (bOk)
                sValue = sSubst;
        }
        catch(const css::container::NoSuchElementException&)
        {
            bOk = sal_False;
        }
    }

    return bOk;
}

// ______________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_5                     (   PathSettings                                             ,
                                            OWeakObject                                              ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider              ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo               ),
                                            DIRECT_INTERFACE( css::beans::XPropertySet              ),
                                            DIRECT_INTERFACE( css::beans::XFastPropertySet          ),
                                            DIRECT_INTERFACE( css::beans::XMultiPropertySet         )
                                        )

DEFINE_XTYPEPROVIDER_5                  (   PathSettings                                            ,
                                            css::lang::XTypeProvider                                ,
                                            css::lang::XServiceInfo                                 ,
                                            css::beans::XPropertySet                                ,
                                            css::beans::XFastPropertySet                            ,
                                            css::beans::XMultiPropertySet
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   PathSettings                                            ,
                                            ::cppu::OWeakObject                                     ,
                                            SERVICENAME_PATHSETTINGS                                ,
                                            IMPLEMENTATIONNAME_PATHSETTINGS
                                        )

DEFINE_INIT_SERVICE                     (   PathSettings,
                                            {
                                                /*Attention
                                                    I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                                                    to create a new instance of this class by our own supported service factory.
                                                    see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                                                */
                                            }
                                        )

/*-************************************************************************************************************//**
    @short      standard constructor to create instance by factory
    @descr      This constructor initialize a new instance of this class by valid factory,
                and will be set valid values on his member and baseclasses.

    @attention  a)  Don't use your own reference during an UNO-Service-ctor! There is no guarantee, that you
                    will get over this. (e.g. using of your reference as parameter to initialize some member)
                    Do such things in DEFINE_INIT_SERVICE() method, which is called automaticly after your ctor!!!
                b)  Baseclass OBroadcastHelper is a typedef in namespace cppu!
                    The microsoft compiler has some problems to handle it right BY using namespace explicitly ::cppu::OBroadcastHelper.
                    If we write it without a namespace or expand the typedef to OBrodcastHelperVar<...> -> it will be OK!?
                    I don't know why! (other compiler not tested .. but it works!)

    @seealso    method DEFINE_INIT_SERVICE()

    @param      "xFactory" is the multi service manager, which create this instance.
                The value must be different from NULL!
    @return     -

    @onerror    We throw an ASSERT in debug version or do nothing in relaese version.
*//*-*************************************************************************************************************/
PathSettings::PathSettings( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
        //  Init baseclasses first
        //  Attention: Don't change order of initialization!
        //      ThreadHelpBase is a struct with a lock as member. We can't use a lock as direct member!
        //      We must garant right initialization and a valid value of this to initialize other baseclasses!
        :   PathSettingsCfg             ( xSMGR                                             )
        ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >           ( m_aLock.getShareableOslMutex()         )
        ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
        ,   ::cppu::OWeakObject         (                                                   )
        // Init member
        ,   m_xSMGR                     ( xSMGR                                             )
{
}

/*-************************************************************************************************************//**
    @short      standard destructor
    @descr      This one do NOTHING! Use dispose() instaed of this.

    @seealso    method dispose()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
PathSettings::~PathSettings()
{
}

/*-************************************************************************************************************//**
    @short      try to convert a property value
    @descr      This method is called from helperclass "OPropertySetHelper".
                Don't use this directly!
                You must try to convert the value of given propertyhandle and
                return results of this operation. This will be used to ask vetoable
                listener. If no listener has a veto, we will change value realy!
                ( in method setFastPropertyValue_NoBroadcast(...) )

    @attention  Methods of OPropertySethelper are safed by using our shared osl mutex! (see ctor!)
                So we must use different locks to make our implementation threadsafe.

    @seealso    class OPropertySetHelper
    @seealso    method setFastPropertyValue_NoBroadcast()
    @seealso    method impl_tryToChangeProperty()

    @param      "aConvertedValue"   new converted value of property
    @param      "aOldValue"         old value of property
    @param      "nHandle"           handle of property
    @param      "aValue"            new value of property
    @return     sal_True if value will be changed, sal_FALSE otherway

    @onerror    IllegalArgumentException, if you call this with an invalid argument
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool SAL_CALL PathSettings::convertFastPropertyValue(   css::uno::Any&          aConvertedValue ,
                                                            css::uno::Any&          aOldValue       ,
                                                            sal_Int32               nHandle         ,
                                                            const css::uno::Any&    aValue          ) throw( css::lang::IllegalArgumentException )
{
    EPropHandle nID = (EPropHandle)nHandle;
    return PropHelper::willPropertyBeChanged(
        css::uno::makeAny(getPath(nID)),
        aValue,
        aOldValue,
        aConvertedValue);
}

/*-************************************************************************************************************//**
    @short      set value of a bound property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!
                Handle and value are valid everyway! You must set the new value only.
                After this, baseclass send messages to all listener automaticly.

    @seealso    class OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    new value of property
    @return     -

    @onerror    An exception is thrown.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL PathSettings::setFastPropertyValue_NoBroadcast(   sal_Int32               nHandle ,
                                                                const css::uno::Any&    aValue  )
    throw( css::uno::Exception )
{
    EPropHandle     nID        = (EPropHandle)nHandle;
    sal_Bool        bReadOnly  = isReadOnly(nID);
    sal_Bool        bMultiPath = isMultiPath(nID);
    ::rtl::OUString sValue     ;

    // Is this value a valid one?
    if (
        (!(aValue >>= sValue                       )) ||
        !(checkAndSubstituteValue(sValue,bMultiPath))
       )
    {
        ::rtl::OUStringBuffer sBuffer(256);
        sBuffer.appendAscii("\""                    );
        sBuffer.append     (sValue                  );
        sBuffer.appendAscii("\" is not a valid URL!");
        throw css::lang::IllegalArgumentException(
                sBuffer.makeStringAndClear(),
                static_cast< cppu::OWeakObject *>(this),
                0);
    }

    // update cache and config
    setPath(nID, sValue);
}

/*-************************************************************************************************************//**
    @short      get value of a bound property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @attention  We don't need any mutex or lock here ... We use threadsafe container or methods here only!

    @seealso    class OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    current value of property
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL PathSettings::getFastPropertyValue(   css::uno::Any& aValue  ,
                                                    sal_Int32      nHandle ) const
{
    PathSettingsCfg::EPropHandle nID = (PathSettingsCfg::EPropHandle)nHandle;
    aValue <<= getPath(nID);
}

/*-************************************************************************************************************//**
    @short      return structure and information about bound properties
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @attention  You must use global lock (method use static variable) ... and it must be the shareable osl mutex of it.
                Because; our baseclass use this mutex to make his code threadsafe. We use our lock!
                So we could have two different mutex/lock mechanism at the same object.

    @seealso    class OPropertySetHelper

    @param      -
    @return     structure with property-informations

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
::cppu::IPropertyArrayHelper& SAL_CALL PathSettings::getInfoHelper()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        // Ready for multithreading
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "getPropertyDescriptor" is a non exported function of our base class PathSettingsCfg
            // which creates the right descriptor on demand and patch against some fix informations
            // e.g. the readonly attribute!
            // Last parameter set to "sal_True" indicates => table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper(getPropertyDescriptor(), sal_True);
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

/*-************************************************************************************************************//**
    @short      return propertysetinfo
    @descr      You can call this method to get information about transient properties
                of this object.

    @attention  You must use global lock (method use static variable) ... and it must be the shareable osl mutex of it.
                Because; our baseclass use this mutex to make his code threadsafe. We use our lock!
                So we could have two different mutex/lock mechanism at the same object.

    @seealso    class OPropertySetHelper
    @seealso    interface XPropertySet
    @seealso    interface XMultiPropertySet

    @param      -
    @return     reference to object with information [XPropertySetInfo]

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL PathSettings::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static css::uno::Reference< css::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

} // namespace framework
