/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: metadata.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:50:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"

#ifndef RPTUI_METADATA_HXX_
#include "metadata.hxx"
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYHANDLER_HPP_
#include "com/sun/star/inspection/XPropertyHandler.hpp"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif

#include <functional>
#include <algorithm>

//............................................................................
namespace rptui
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star;

    //========================================================================
    //= OPropertyInfoImpl
    //========================================================================
    struct OPropertyInfoImpl
    {
        String          sName;
        String          sTranslation;
        sal_uInt32      nHelpId;
        sal_Int32       nId;
        sal_uInt16      nPos;
        sal_uInt32      nUIFlags;

        OPropertyInfoImpl(
                        const ::rtl::OUString&      rName,
                        sal_Int32                   _nId,
                        const String&               aTranslation,
                        sal_uInt16                  nPosId,
                        sal_uInt32                  nHelpId,
                        sal_uInt32                  _nUIFlags);
    };

    //------------------------------------------------------------------------
    OPropertyInfoImpl::OPropertyInfoImpl(const ::rtl::OUString& _rName, sal_Int32 _nId,
                                   const String& aString, sal_uInt16 nP, sal_uInt32 nHid, sal_uInt32 _nUIFlags)
       :sName(_rName)
       ,sTranslation(aString)
       ,nHelpId(nHid)
       ,nId(_nId)
       ,nPos(nP)
       ,nUIFlags(_nUIFlags)
    {
    }

    //------------------------------------------------------------------------
    // Vergleichen von PropertyInfo
    struct PropertyInfoLessByName : public ::std::binary_function< OPropertyInfoImpl, OPropertyInfoImpl, bool >
    {
        bool operator()( const OPropertyInfoImpl& _lhs, const OPropertyInfoImpl& _rhs )
        {
            return _lhs.sName < _rhs.sName;
        }
    };

    //========================================================================
    //= OPropertyInfoService
    //========================================================================
#define DEF_INFO( ident, uinameres, helpid, flags )   \
    OPropertyInfoImpl( PROPERTY_##ident, PROPERTY_ID_##ident, \
            String( ModuleRes( RID_STR_##uinameres ) ), nPos++, HID_RPT_PROP_##helpid, flags )

#define DEF_INFO_1( ident, uinameres, helpid, flag1 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 )

#define DEF_INFO_2( ident, uinameres, helpid, flag1, flag2 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 )

#define DEF_INFO_3( ident, uinameres, helpid, flag1, flag2, flag3 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 )

#define DEF_INFO_4( ident, uinameres, helpid, flag1, flag2, flag3, flag4 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 | PROP_FLAG_##flag4 )

#define DEF_INFO_5( ident, uinameres, helpid, flag1, flag2, flag3, flag4, flag5 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 | PROP_FLAG_##flag4 | PROP_FLAG_##flag5 )

    sal_uInt16              OPropertyInfoService::s_nCount = 0;
    OPropertyInfoImpl*      OPropertyInfoService::s_pPropertyInfos = NULL;
    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo()
    {
        if ( s_pPropertyInfos )
            return s_pPropertyInfos;

        OModuleClient aResourceAccess;
        // this ensures that we have our resource file loaded

        sal_uInt16 nPos = 1;

        static OPropertyInfoImpl aPropertyInfos[] =
        {
        /*
        DEF_INFO_?( propname and id,   resoure id,         help id,           flags ),
        */
             DEF_INFO_1( FORCENEWPAGE,                  FORCENEWPAGE,               FORCENEWPAGE,               COMPOSEABLE )
            ,DEF_INFO_1( NEWROWORCOL,                   NEWROWORCOL,                NEWROWORCOL,                COMPOSEABLE )
            ,DEF_INFO_1( KEEPTOGETHER,                  KEEPTOGETHER,               KEEPTOGETHER,               COMPOSEABLE )
            ,DEF_INFO_1( CANGROW,                       CANGROW,                    CANGROW,                    COMPOSEABLE )
            ,DEF_INFO_1( CANSHRINK,                     CANSHRINK,                  CANSHRINK,                  COMPOSEABLE )
            ,DEF_INFO_1( REPEATSECTION,                 REPEATSECTION,              REPEATSECTION,              COMPOSEABLE )
            ,DEF_INFO_1( PRINTREPEATEDVALUES,           PRINTREPEATEDVALUES,        PRINTREPEATEDVALUES,        COMPOSEABLE )
            ,DEF_INFO_1( CONDITIONALPRINTEXPRESSION,    CONDITIONALPRINTEXPRESSION, CONDITIONALPRINTEXPRESSION, COMPOSEABLE )
            ,DEF_INFO_1( STARTNEWCOLUMN,                STARTNEWCOLUMN,             STARTNEWCOLUMN,             COMPOSEABLE )
            ,DEF_INFO_1( RESETPAGENUMBER,               RESETPAGENUMBER,            RESETPAGENUMBER,            COMPOSEABLE )
            ,DEF_INFO_1( PRINTWHENGROUPCHANGE,          PRINTWHENGROUPCHANGE,       PRINTWHENGROUPCHANGE,       COMPOSEABLE )
            ,DEF_INFO_1( VISIBLE,                       VISIBLE,                    VISIBLE,                    COMPOSEABLE )
            ,DEF_INFO_1( GROUPKEEPTOGETHER,             GROUPKEEPTOGETHER,          GROUPKEEPTOGETHER,          COMPOSEABLE )
            ,DEF_INFO_1( PAGEHEADEROPTION,              PAGEHEADEROPTION,           PAGEHEADEROPTION,           COMPOSEABLE )
            ,DEF_INFO_1( PAGEFOOTEROPTION,              PAGEFOOTEROPTION,           PAGEFOOTEROPTION,           COMPOSEABLE )
            ,DEF_INFO_1( POSITIONX,                     POSITIONX,                  RPT_POSITIONX,              COMPOSEABLE )
            ,DEF_INFO_1( POSITIONY,                     POSITIONY,                  RPT_POSITIONY,              COMPOSEABLE )
            ,DEF_INFO_1( WIDTH,                         WIDTH,                      RPT_WIDTH,                  COMPOSEABLE )
            ,DEF_INFO_1( HEIGHT,                        HEIGHT,                     RPT_HEIGHT,                 COMPOSEABLE )
            ,DEF_INFO_1( CHARFONTNAME,                  FONT,                       RPT_FONT,                   COMPOSEABLE )
            ,DEF_INFO_1( PREEVALUATED,                  PREEVALUATED,               PREEVALUATED,               COMPOSEABLE )
            ,DEF_INFO_1( DEEPTRAVERSING,                DEEPTRAVERSING,             DEEPTRAVERSING,             COMPOSEABLE )
            ,DEF_INFO_1( FORMULA,                       FORMULA,                    FORMULA,                    COMPOSEABLE )
            ,DEF_INFO_1( INITIALFORMULA,                INITIALFORMULA,             INITIALFORMULA,             COMPOSEABLE )
            ,DEF_INFO_2( TYPE,                          TYPE,                       TYPE,                       COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_2( DATAFIELD,                     DATAFIELD,                  DATAFIELD,                  COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_2( FORMULALIST,                   FORMULALIST,                FORMULALIST,                COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_2( SCOPE,                         SCOPE,                      SCOPE,                      COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_1( PRESERVEIRI,                   PRESERVEIRI,                PRESERVEIRI,                COMPOSEABLE )
            ,DEF_INFO_1( BACKCOLOR,                     BACKCOLOR,                  BACKCOLOR,                  COMPOSEABLE )
            ,DEF_INFO_1( CONTROLBACKGROUND,             BACKCOLOR,                  BACKCOLOR,                  COMPOSEABLE )
            ,DEF_INFO_1( BACKTRANSPARENT,               BACKTRANSPARENT,            BACKTRANSPARENT,            COMPOSEABLE )
            ,DEF_INFO_1( CONTROLBACKGROUNDTRANSPARENT,  CONTROLBACKGROUNDTRANSPARENT
                                                                    ,CONTROLBACKGROUNDTRANSPARENT, COMPOSEABLE )
        };

        s_pPropertyInfos = aPropertyInfos;
        s_nCount = sizeof(aPropertyInfos) / sizeof(OPropertyInfoImpl);

        ::std::sort( aPropertyInfos, aPropertyInfos + s_nCount, PropertyInfoLessByName() );

        return s_pPropertyInfos;
    }

    //------------------------------------------------------------------------
    sal_Int32 OPropertyInfoService::getPropertyId(const String& _rName) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_rName);
        return pInfo ? pInfo->nId : -1;
    }

    //------------------------------------------------------------------------
    String OPropertyInfoService::getPropertyName( sal_Int32 _nPropId )
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nPropId);
        return pInfo ? pInfo->sName : String();
    }

    //------------------------------------------------------------------------
    String OPropertyInfoService::getPropertyTranslation(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->sTranslation : String();
    }

    //------------------------------------------------------------------------
    sal_Int32 OPropertyInfoService::getPropertyHelpId(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nHelpId : 0;
    }

    //------------------------------------------------------------------------
    sal_Int16 OPropertyInfoService::getPropertyPos(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nPos : 0xFFFF;
    }

    //------------------------------------------------------------------------
    sal_uInt32 OPropertyInfoService::getPropertyUIFlags(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nUIFlags : 0;
    }

    //------------------------------------------------------------------------
    void OPropertyInfoService::getPropertyEnumRepresentations(sal_Int32 _nId,::std::vector< ::rtl::OUString >& _rOut) const
    {
        OSL_ENSURE( ( ( getPropertyUIFlags( _nId ) & PROP_FLAG_ENUM ) != 0 ) ,
            "OPropertyInfoService::getPropertyEnumRepresentations: this is no enum property!" );

        sal_Int16 nCommaSeparatedListResId = 0;
        sal_Int16 nStringItemsResId = 0;
        switch ( _nId )
        {
            case PROPERTY_ID_FORCENEWPAGE:
                nCommaSeparatedListResId = RID_STR_FORCENEWPAGE_CONST;
                break;
        }

        if ( nCommaSeparatedListResId )
        {
            String sSeparatedList = String( ModuleRes( nCommaSeparatedListResId ) );
            xub_StrLen nTokens = sSeparatedList.GetTokenCount();
            _rOut.reserve( nTokens );
            for ( xub_StrLen i = 0; i < nTokens; ++i )
                _rOut.push_back( sSeparatedList.GetToken( i ) );
        }
        else if ( nStringItemsResId )
        {
            ModuleRes aModuleRes( nStringItemsResId );
            ::svt::OLocalResourceAccess aEnumStrings( aModuleRes, RSC_RESOURCE );

            sal_Int16 i = 1;
            ModuleRes aLocalId( i );
            while ( aEnumStrings.IsAvailableRes( aLocalId.SetRT( RSC_STRING ) ) )
            {
                _rOut.push_back( String( aLocalId ) );
                aLocalId = ModuleRes( ++i );
            }
        }
    }

    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(const String& _rName)
    {
        // intialisierung
        if(!s_pPropertyInfos)
            getPropertyInfo();
        OPropertyInfoImpl  aSearch(_rName, 0L, String(), 0, 0, 0);

        const OPropertyInfoImpl* pPropInfo = ::std::lower_bound(
            s_pPropertyInfos, s_pPropertyInfos + s_nCount, aSearch, PropertyInfoLessByName() );

        if ( ( pPropInfo < s_pPropertyInfos + s_nCount ) && pPropInfo->sName == _rName )
            return pPropInfo;

        return NULL;
    }


    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(sal_Int32 _nId)
    {
        // intialisierung
        if(!s_pPropertyInfos)
            getPropertyInfo();

        // TODO: a real structure which allows quick access by name as well as by id
        for (sal_uInt16 i = 0; i < s_nCount; i++)
            if (s_pPropertyInfos[i].nId == _nId)
                return &s_pPropertyInfos[i];

        return NULL;
    }

    //------------------------------------------------------------------------
    bool OPropertyInfoService::isComposable( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >& _rxFormComponentHandler )
    {
        sal_Int32 nId = getPropertyId( _rPropertyName );
        if ( nId != -1 )
        {
            sal_uInt32 nFlags = getPropertyUIFlags( nId );
            return ( nFlags & PROP_FLAG_COMPOSEABLE ) != 0;
        }

        return _rxFormComponentHandler->isComposable( _rPropertyName );
    }

    //------------------------------------------------------------------------
    void OPropertyInfoService::getExcludeProperties(::std::vector< beans::Property >& _rExcludeProperties,const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >& _xFormComponentHandler)
    {
        uno::Sequence< beans::Property > aProps = _xFormComponentHandler->getSupportedProperties();
        ::rtl::OUString pExcludeProperties[] =
        {
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Enabled")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Printable")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WordBreak")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MultiLine")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Tag")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HelpText")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HelpURL")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MaxTextLen")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Tabstop")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TabIndex")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ValueMin")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ValueMax")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Spin")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SpinValue")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SpinValueMin")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SpinValueMax")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultSpinValue")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SpinIncrement")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Repeat")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RepeatDelay")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlLabel")), /// TODO: has to be checked
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LabelControl")),
                PROPERTY_MAXTEXTLEN,
                PROPERTY_EFFECTIVEDEFAULT,
                PROPERTY_EFFECTIVEMAX,
                PROPERTY_EFFECTIVEMIN,
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HideInactiveSelection")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SubmitAction")),
                PROPERTY_EMPTY_IS_NULL,
                PROPERTY_FILTERPROPOSAL
                ,PROPERTY_POSITIONX
                ,PROPERTY_POSITIONY
                ,PROPERTY_WIDTH
                ,PROPERTY_HEIGHT
                ,PROPERTY_FONTNAME
                ,PROPERTY_LABEL
                ,PROPERTY_LINECOLOR
                ,PROPERTY_BORDER
                ,PROPERTY_BORDERCOLOR
                ,PROPERTY_BACKTRANSPARENT
                ,PROPERTY_CONTROLBACKGROUND
                ,PROPERTY_BACKGROUNDCOLOR
                ,PROPERTY_CONTROLBACKGROUNDTRANSPARENT
                ,PROPERTY_FORMULALIST
                ,PROPERTY_SCOPE
                ,PROPERTY_TYPE
        };

        beans::Property* pPropsIter = aProps.getArray();
        beans::Property* pPropsEnd = pPropsIter + aProps.getLength();
        for (; pPropsIter != pPropsEnd; ++pPropsIter)
        {
            size_t nPos = 0;
            for (; nPos < sizeof(pExcludeProperties)/sizeof(pExcludeProperties[0]) && pExcludeProperties[nPos] != pPropsIter->Name;++nPos )
                ;
            if ( nPos == sizeof(pExcludeProperties)/sizeof(pExcludeProperties[0]) )
                _rExcludeProperties.push_back(*pPropsIter);
        }
    }


//............................................................................
} // namespace pcr
//............................................................................

