/*************************************************************************
 *
 *  $RCSfile: fltfnc.cxx,v $
 *
 *  $Revision: 1.53 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:57:51 $
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

#include "fltfnc.hxx"

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif

#ifndef _EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <basic/sbmeth.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <svtools/sbxobj.hxx>
#endif
#ifndef __SBX_SBXMETHOD_HXX //autogen
#include <svtools/sbxmeth.hxx>
#endif
#ifndef _SBXCORE_HXX //autogen
#include <svtools/sbxcore.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _RTL_USTRING_HXX //autogen
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _EXTATTR_HXX
#include <svtools/extattr.hxx>
#endif
#ifndef _LCKBITEM_HXX //autogen
#include <svtools/lckbitem.hxx>
#endif
#ifndef _INETTYPE_HXX //autogen
#include <svtools/inettype.hxx>
#endif
#ifndef _SFXRECTITEM_HXX
#include <svtools/rectitem.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSTANDALONEDOCUMENTINFO_HPP_
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HPP_
#include <com/sun/star/frame/FrameActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADEVENTLISTENER_HPP_
#include <com/sun/star/frame/XLoadEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFILTERDETECT_HPP_
#include <com/sun/star/frame/XFilterDetect.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/ucb/XContent.hpp>
#include <rtl/ustring.hxx>
#include <vos/process.hxx>
#include <svtools/pathoptions.hxx>

#ifndef _L2TXTENC_HXX
#include <tools/l2txtenc.hxx>
#endif

#include <rtl/logfile.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::vos;

#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _SVTOOLS_CTYPEITM_HXX
#include <svtools/ctypeitm.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif

#include "app.hxx"
#include "sfxhelp.hxx"
#include "sfxbasic.hxx"
#include "docfilt.hxx"
#include "docfac.hxx"
#include "sfxtypes.hxx"
#include "sfxuno.hxx"
#include "docfile.hxx"
#include "progress.hxx"
#include "loadenv.hxx"
#include "openflag.hxx"
#include "bastyp.hrc"
#include "sfxresid.hxx"
#include "doctempl.hxx"
#include "frame.hxx"
#include "dispatch.hxx"
#include "urlframe.hxx"
#include "topfrm.hxx"
#include "plugwin.hxx"
#include "helper.hxx"
#include "fltlst.hxx"
#include "request.hxx"
#include "nfltdlg.hxx"
#include "arrdecl.hxx"
#include "appdata.hxx"
#include "appuno.hxx"
#include "viewfrm.hxx"

struct FlagMapping_Impl
{
    const char* pName;
    SfxFilterFlags nValue;
};

class SfxFilterContainerArr_Impl;
static SfxFilterContainerArr_Impl* pContArr = 0;
static SfxFilterList_Impl* pFilterArr = 0;

//----------------------------------------------------------------
inline String ToUpper_Impl( const String &rStr )
{
    return SvtSysLocale().GetCharClass().upper( rStr );
}

const SfxFilter*    GetFilter4Mime( SfxFilterList_Impl& rList, const String& rMime, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED );
const SfxFilter*    GetFilter4ClipBoardId( SfxFilterList_Impl& rList, ULONG nId, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED );
const SfxFilter*    GetFilter4EA( SfxFilterList_Impl& rList, const String& rEA, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED );
const SfxFilter*    GetFilter4Extension( SfxFilterList_Impl& rList, const String& rExt, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED );
const SfxFilter*    GetFilter4FilterName( SfxFilterList_Impl& rList, const String& rName, SfxFilterFlags nMust = 0, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED );
const SfxFilter*    GetFilter4UIName( SfxFilterList_Impl& rList, const String& rName, SfxFilterFlags nMust = 0, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED );

//----------------------------------------------------------------
class SfxFilterContainer_Impl
{
public:
    SfxFilterList_Impl      aList;
    String                  aName;
    String                  aServiceName;
    SfxFilterContainerFlags eFlags;

                            SfxFilterContainer_Impl( const String& rName )
                                : aName( rName )
                                , eFlags( 0 )
                                {
                                    aServiceName = SfxObjectShell::GetServiceNameFromFactory( rName );
                                }

    sal_Bool            equalFilterNames( const String& s1, const ::rtl::OUString& s2 ) const;
    static SfxFilterContainer_Impl* Get( const String& rName, BOOL bCreate=TRUE );
    void                Update();
};

void SfxFilterContainer_Impl::Update()
{
    aList.Clear();
    for ( USHORT n=0; n<pFilterArr->Count(); n++ )
    {
        SfxFilter* pFilter = pFilterArr->GetObject(n);
        if ( pFilter->GetServiceName() == aServiceName )
        aList.Insert( pFilter, LIST_APPEND );
    }
}

DECL_PTRARRAY( SfxFilterContainerArr_Impl, SfxFilterContainer_Impl*, 2, 2 );

SfxFilterContainer_Impl* SfxFilterContainer_Impl::Get( const String& rName, BOOL bCreate )
{
    if ( !pFilterArr )
        SfxFilterContainer::ReadFilters_Impl();

    if ( !pContArr )
        pContArr = new SfxFilterContainerArr_Impl;

    SfxFilterContainer_Impl* pImpl = 0;
    for ( USHORT n=0; n<pContArr->Count(); n++ )
    {
        pImpl = pContArr->GetObject(n);
        if ( pImpl->aName == rName )
            return pContArr->GetObject(n);
    }

    if ( bCreate )
    {
        pImpl = new SfxFilterContainer_Impl( rName );
        pContArr->Insert( pContArr->Count(), pImpl );
        pImpl->Update();
        return pImpl;
    }

    return NULL;
}

//----------------------------------------------------------------

SfxFilterContainer::SfxFilterContainer( const String& rName )
{
    pImpl = SfxFilterContainer_Impl::Get( rName );
}

//----------------------------------------------------------------

SfxFilterContainer::~SfxFilterContainer()
{
/*
    SfxFilterList_Impl& rList = pImpl->aList;
    sal_uInt16 nCount = (sal_uInt16 )rList.Count();
    for( sal_uInt16 n = 0; n<nCount; n++ )
        delete rList.GetObject( n );
    pContArr->Remove( pImpl );
    delete pImpl;
 */
}

SfxFilterContainerFlags SfxFilterContainer::GetFlags() const
{
    return pImpl->eFlags;
}

void SfxFilterContainer::SetFlags( SfxFilterContainerFlags eFlags )
{
    pImpl->eFlags = eFlags;
}

//----------------------------------------------------------------

sal_uInt16 SfxFilterContainer::GetFilterCount() const
{
    return (sal_uInt16) pImpl->aList.Count();
}

const SfxFilter* SfxFilterContainer::GetFilter( sal_uInt16 nPos ) const
{
    return pImpl->aList.GetObject( nPos );
}

const SfxFilter* SfxFilterContainer::GetAnyFilter( SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    sal_uInt16 nCount = ( sal_uInt16 ) pImpl->aList.Count();
    for( sal_uInt16 n = 0; n < nCount; n++ )
    {
        const SfxFilter* pFilter = pImpl->aList.GetObject( n );
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) )
            return pFilter;
    }

    return NULL;
}

#define IMPL_FORWARD_LOOP( aMethod, ArgType, aArg )         \
const SfxFilter* SfxFilterContainer::aMethod(                   \
    ArgType aArg, SfxFilterFlags nMust, SfxFilterFlags nDont ) const \
{                                                               \
    return ::aMethod( pImpl->aList, aArg, nMust, nDont );   \
}

IMPL_FORWARD_LOOP( GetFilter4Mime, const String&, rMime );
IMPL_FORWARD_LOOP( GetFilter4ClipBoardId, ULONG, nId );
IMPL_FORWARD_LOOP( GetFilter4EA, const String&, rEA );
IMPL_FORWARD_LOOP( GetFilter4Extension, const String&, rExt );
IMPL_FORWARD_LOOP( GetFilter4FilterName, const String&, rName );
IMPL_FORWARD_LOOP( GetFilter4UIName, const String&, rName );

const SfxFilter* GetFilter4FilterName( SfxFilterList_Impl& rList, const String& rName, SfxFilterFlags nMust, SfxFilterFlags nDont )
{
    String aName( rName );
    USHORT nIndex = aName.SearchAscii(": ");
    if (  nIndex != STRING_NOTFOUND )
    {
        DBG_ERROR("Old filter name used!");
        aName = rName.Copy( nIndex + 2 );
    }

    const SfxFilter* pFirstFilter=0;
    sal_uInt16 nCount = ( sal_uInt16 ) rList.Count();
    for( sal_uInt16 n = 0; n < nCount; n++ )
    {
        const SfxFilter* pFilter = rList.GetObject( n );
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if ( (nFlags & nMust) == nMust &&
             !(nFlags & nDont ) && pFilter->GetFilterName().CompareIgnoreCaseToAscii( aName ) == COMPARE_EQUAL )
        {
            if ( pFilter->GetFilterFlags() & SFX_FILTER_PREFERED )
                return pFilter;
            else if ( !pFirstFilter )
                pFirstFilter = pFilter;
        }
    }
    return pFirstFilter;
}

//----------------------------------------------------------------
/*   [Beschreibung]

     Da die meisten GetFilter4xxx Routinen in einem Container einfach eine
     Abfrage ueber alle enthaltenen Filter durchlaufen, sind diese in
     einem Makro kodiert.

     nMust sind die Flags, die gesetzt sein muessen, damit der Filter
     in Betracht gezogen wird, nDont duerfen nicht gesetzt sein.
 */

#define IMPL_LIST_LOOP( aMethod, aArgType, aTest )         \
const SfxFilter* aMethod( SfxFilterList_Impl& rList,            \
    aArgType aArg, SfxFilterFlags nMust, SfxFilterFlags nDont ) \
{                                                               \
    const SfxFilter* pFirstFilter=0;                            \
    sal_uInt16 nCount = ( sal_uInt16 ) rList.Count();           \
    for( sal_uInt16 n = 0; n < nCount; n++ )                    \
    {                                                           \
        const SfxFilter* pFilter = rList.GetObject( n );        \
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();      \
        if ( (nFlags & nMust) == nMust &&                       \
             !(nFlags & nDont ) && aTest )                      \
        {                                                       \
            if ( pFilter->GetFilterFlags() & SFX_FILTER_PREFERED ) \
                return pFilter;                                 \
            else if ( !pFirstFilter )                           \
                pFirstFilter = pFilter;                         \
        }                                                       \
    }                                                           \
    return pFirstFilter;                                        \
}

IMPL_LIST_LOOP(
    GetFilter4Mime, const String&,
    pFilter->GetMimeType().CompareIgnoreCaseToAscii( aArg ) == COMPARE_EQUAL )

IMPL_LIST_LOOP(
    GetFilter4ClipBoardId, sal_uInt32, aArg && pFilter->GetFormat() == aArg )

#ifdef MAC
#define CONDITION \
    String( aArg ).Erase( 5 ) == String( pFilter->GetTypeName() ).Erase( 5 )
#else
#define CONDITION \
    aArg == pFilter->GetTypeName()
#endif

IMPL_LIST_LOOP(
    GetFilter4EA, const String&, CONDITION )
#undef CONDITION

IMPL_LIST_LOOP(
    GetFilter4Extension, const String&,
    pFilter->GetWildcard() != String() && pFilter->GetWildcard() != DEFINE_CONST_UNICODE("*.*") && pFilter->GetWildcard() != 0x002A &&
    WildCard( ToUpper_Impl( pFilter->GetWildcard()() ), ';' ) == ToUpper_Impl( aArg ))

IMPL_LIST_LOOP(
    GetFilter4UIName, const String&,
    pFilter->GetUIName() == aArg )

//----------------------------------------------------------------

const String SfxFilterContainer::GetName() const
{
    return pImpl->aName;
}

//----------------------------------------------------------------

void SfxFilterContainer::AddFilter_Impl( SfxFilter* pFilter, sal_uInt16 nPos )
{
    pImpl->aList.Insert( pFilter, nPos );
}

//-------------------------------------------------------------------------

DECLARE_LIST( SfxFContainerList_Impl, SfxFilterContainer * );

class SfxFilterMatcher_Impl
{
public:
    SfxFilterList_Impl* pList;
    SfxFilterContainer_Impl* pCont;
};

//----------------------------------------------------------------

SfxFilterMatcher::SfxFilterMatcher( const String& rName )
{
    pImpl = new SfxFilterMatcher_Impl;
    pImpl->pCont = SfxFilterContainer_Impl::Get( rName );
    pImpl->pList = &pImpl->pCont->aList;
}

//----------------------------------------------------------------

SfxFilterMatcher::SfxFilterMatcher()
{
    pImpl = new SfxFilterMatcher_Impl;
    if ( !pFilterArr )
        SfxFilterContainer::ReadFilters_Impl();
    pImpl->pList = pFilterArr;
    pImpl->pCont = 0;
}

SfxFilterMatcher::~SfxFilterMatcher()
{
    delete pImpl;
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilterIgnoringContent( SfxMedium& rMedium, const SfxFilter**ppFilter, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    Reference< XTypeDetection > xDetection( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.document.TypeDetection")), UNO_QUERY );
    ::rtl::OUString sTypeName;
    try
    {
        //!MBA: nmust, ndont?
        sTypeName = xDetection->queryTypeByURL( rMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
    }
    catch( Exception& )
    {
    }

    *ppFilter = NULL;
    if ( sTypeName.getLength() )
        *ppFilter = GetFilter4EA( sTypeName );

    return *ppFilter ? ERRCODE_NONE : ERRCODE_ABORT;
}

//----------------------------------------------------------------

#define CHECKERROR()                                            \
if( nErr == 1 || nErr == USHRT_MAX || nErr == ULONG_MAX )       \
{                                                               \
    ByteString aText = "Fehler in FilterDetection: Returnwert ";\
    aText += ByteString::CreateFromInt32(nErr);                 \
    if( pFilter )                                               \
    {                                                           \
        aText += ' ';                                           \
        aText += ByteString(U2S(pFilter->GetFilterName()));     \
    }                                                           \
    DBG_ERROR( aText.GetBuffer() );                             \
    nErr = ERRCODE_ABORT;                                       \
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilter( SfxMedium& rMedium, const SfxFilter**ppFilter, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    const SfxFilter* pOldFilter = *ppFilter;
    const SfxFilter* pFilter = pOldFilter;

    rMedium.UseInteractionHandler(TRUE);

    Reference< XTypeDetection > xDetection( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.document.TypeDetection")), UNO_QUERY );
    Sequence< PropertyValue > aDescriptor(3);
    aDescriptor[0].Name = ::rtl::OUString::createFromAscii("InputStream");
    aDescriptor[0].Value <<= rMedium.GetInputStream();
    aDescriptor[1].Name = ::rtl::OUString::createFromAscii("InteractionHandler");
    aDescriptor[1].Value <<= rMedium.GetInteractionHandler();
    aDescriptor[2].Name = ::rtl::OUString::createFromAscii("FileName");
    aDescriptor[2].Value <<= ::rtl::OUString( rMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );

    if ( pImpl->pCont )
    {
        aDescriptor.realloc(4);
        aDescriptor[3].Name = ::rtl::OUString::createFromAscii("DocumentServiceName");
        aDescriptor[3].Value <<= ::rtl::OUString( SfxObjectShell::GetServiceNameFromFactory( pImpl->pCont->aName ) );
    }

    ::rtl::OUString sTypeName;
    try
    {
        //!MBA: nmust, ndont?
        sTypeName = xDetection->queryTypeByDescriptor(aDescriptor,sal_True);
    }
    catch( Exception& )
    {
    }

    if ( sTypeName.getLength() )
    {
        String aFilterName;
        sal_uInt32 nPropertyCount = aDescriptor.getLength();
        for( sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
        {
            if( aDescriptor[nProperty].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
            {
                // the name of the desired filter, usually to prevent us from using the default
                // filter for the detected type
                ::rtl::OUString sTemp;
                aDescriptor[nProperty].Value >>= sTemp;
                aFilterName = sTemp;
            }
        }

        if ( aFilterName.Len() )
        {
            *ppFilter = SfxFilter::GetFilterByName( aFilterName );
            return ERRCODE_NONE;
        }
    }

    return ERRCODE_ABORT;
}

//----------------------------------------------------------------
sal_Bool SfxFilterMatcher::IsFilterInstalled_Impl( const SfxFilter* pFilter )
{
    Window *pWindow = SFX_APP()->GetTopWindow();
    if ( pFilter->GetFilterFlags() & SFX_FILTER_MUSTINSTALL )
    {
        // Hier k"onnte noch eine Nachinstallation angeboten werden
        String aText( SfxResId( STR_FILTER_NOT_INSTALLED ) );
        aText.SearchAndReplaceAscii( "$(FILTER)", pFilter->GetUIName() );
        QueryBox aQuery( pWindow, WB_YES_NO | WB_DEF_YES, aText );
        short nRet = aQuery.Execute();
        if ( nRet == RET_YES )
        {
#ifdef DBG_UTIL
            // Setup starten
            InfoBox( pWindow, DEFINE_CONST_UNICODE("Hier soll jetzt das Setup starten!") ).Execute();
#endif
            // Installation mu\s hier noch mitteilen, ob es geklappt hat, dann kann das
            // Filterflag gel"oscht werden
        }

        return ( !(pFilter->GetFilterFlags() & SFX_FILTER_MUSTINSTALL) );
    }
    else if ( pFilter->GetFilterFlags() & SFX_FILTER_CONSULTSERVICE )
    {
        String aText( SfxResId( STR_FILTER_CONSULT_SERVICE ) );
        aText.SearchAndReplaceAscii( "$(FILTER)", pFilter->GetUIName() );
        InfoBox ( pWindow, aText ).Execute();
        return sal_False;
    }
    else
        return sal_True;
}


sal_uInt32 SfxFilterMatcher::DetectFilter( SfxMedium& rMedium, const SfxFilter**ppFilter, sal_Bool bPlugIn, sal_Bool bAPI ) const
/*  [Beschreibung]

    Hier wird noch die Filterauswahlbox hochgezogen. Sonst GuessFilter
 */

{
    const SfxFilter* pOldFilter = rMedium.GetFilter();
    if ( pOldFilter )
    {
        if( !IsFilterInstalled_Impl( pOldFilter ) )
            pOldFilter = 0;
        else
        {
            SFX_ITEMSET_ARG( rMedium.GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
            if ( ( pOldFilter->GetFilterFlags() & SFX_FILTER_PACKED ) && pSalvageItem )
                // Salvage is always done without packing
                pOldFilter = 0;
        }
    }

    const SfxFilter* pFilter = pOldFilter;

    sal_Bool bPreview = rMedium.IsPreview_Impl(), bInsert = sal_False;
    SFX_ITEMSET_ARG(rMedium.GetItemSet(), pReferer, SfxStringItem, SID_REFERER, FALSE);
    if ( bPreview && rMedium.IsRemote() && ( !pReferer || pReferer->GetValue().CompareToAscii("private:searchfolder:",21 ) != COMPARE_EQUAL ) )
        return ERRCODE_ABORT;

    ErrCode nErr = GuessFilter( rMedium, &pFilter );
    if ( nErr == ERRCODE_ABORT )
        return nErr;

    if ( nErr == ERRCODE_IO_PENDING )
    {
        *ppFilter = pFilter;
        return nErr;
    }

    if ( !pFilter )
    {
        const SfxFilter* pInstallFilter = NULL;

        // Jetzt auch Filter testen, die nicht installiert sind ( ErrCode ist irrelevant )
        GuessFilter( rMedium, &pInstallFilter, SFX_FILTER_IMPORT, SFX_FILTER_CONSULTSERVICE );
        if ( pInstallFilter )
        {
            if ( IsFilterInstalled_Impl( pInstallFilter ) )
                // Eventuell wurde der Filter nachinstalliert
                pFilter = pInstallFilter;
        }
        else
        {
            // Jetzt auch Filter testen, die erst von Star bezogen werden m"ussen ( ErrCode ist irrelevant )
            GuessFilter( rMedium, &pInstallFilter, SFX_FILTER_IMPORT, 0 );
            if ( pInstallFilter )
                IsFilterInstalled_Impl( pInstallFilter );
        }
    }

    sal_Bool bHidden = bPreview;
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pFlags, SfxStringItem, SID_OPTIONS, sal_False);
    if ( !bHidden && pFlags )
    {
        String aFlags( pFlags->GetValue() );
        aFlags.ToUpperAscii();
        if( STRING_NOTFOUND != aFlags.Search( 'H' ) )
            bHidden = sal_True;
    }
/*
    if ( ( !pFilter || nErr == ERRCODE_SFX_CONSULTUSER ) && !bAPI && !bHidden )
    {
        if ( !pFilter )
            pFilter = pOldFilter;

        String aTmpName;
        if ( pFilter )
            aTmpName = pFilter->GetUIName();

        SfxFilterMatcher *pMatcher;
        if( bPlugIn && pFilter )
            pMatcher = new SfxFilterMatcher( (SfxFilterContainer *) pFilter->GetFilterContainer() );
        else
            pMatcher = (SfxFilterMatcher*) this;

        SfxFilterDialog *pDlg = new SfxFilterDialog( 0, &rMedium, *pMatcher, pFilter ? &aTmpName: 0, 0 );
        const sal_Bool bOk = RET_OK == pDlg->Execute();
        if (bOk)
            pFilter  = pMatcher->GetFilter4UIName( pDlg->GetSelectEntry());

        if( bPlugIn && pFilter )
            delete pMatcher;
        delete pDlg;

        if ( !bOk)
            nErr = ERRCODE_ABORT;
        else
            nErr = ERRCODE_NONE;
    }
*/
    *ppFilter = pFilter;

    if ( bHidden || bAPI && nErr == ERRCODE_SFX_CONSULTUSER )
        nErr = pFilter ? ERRCODE_NONE : ERRCODE_ABORT;
    return nErr;
}

#define IMPL_LOOP( Type, ArgType )                              \
const SfxFilter* SfxFilterMatcher::Type(                        \
    ArgType rStr, SfxFilterFlags nMust, SfxFilterFlags nDont ) const \
{                                                               \
    const SfxFilter* pFirstFilter = 0;                          \
    SfxApplication* pApp = SFX_APP();                           \
    SfxFilterList_Impl& rList = *pImpl->pList;                      \
    return ::Type( rList, rStr, nMust, nDont );                 \
}

/*
    sal_uInt16 nCount = (sal_uInt16)rList.Count();              \
    for( sal_uInt16 n = 0; n<nCount; n++ )                      \
    {                                                           \
        const SfxFilter* pFilter =                              \
            rList.GetObject( n )->Type(rStr, nMust, nDont );    \
        if( pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_PREFERED ) ) \
            return pFilter;                                     \
        else if ( !pFirstFilter )                               \
            pFirstFilter = pFilter;                             \
    }                                                           \
    return pFirstFilter;                                        \
 */

IMPL_LOOP( GetFilter4Mime, const String& )
IMPL_LOOP( GetFilter4EA, const String& )
IMPL_LOOP( GetFilter4Extension, const String& )
IMPL_LOOP( GetFilter4ClipBoardId, sal_uInt32 )
IMPL_LOOP( GetFilter4FilterName, const String& )
IMPL_LOOP( GetFilter4UIName, const String& )

IMPL_STATIC_LINK( SfxFilterMatcher, MaybeFileHdl_Impl, String*, pString )
{
    const SfxFilter* pFilter =
        pThis->GetFilter4Extension( *pString, SFX_FILTER_IMPORT );
    if( pFilter && !pFilter->GetWildcard().Matches( String() ) &&
        pFilter->GetWildcard() != DEFINE_CONST_UNICODE("*.*") && pFilter->GetWildcard() != '*' )
        return sal_True;
    return sal_False;
}

//----------------------------------------------------------------

SfxFilterMatcherIter::SfxFilterMatcherIter(
    const SfxFilterMatcher* pMatchP,
    SfxFilterFlags nOrMaskP, SfxFilterFlags nAndMaskP )
    : pMatch( pMatchP->pImpl)
    , nOrMask( nOrMaskP ), nAndMask( nAndMaskP )
    , nCurrent(0)
{
    // Iterator auf AppFilterMatcher -> DoInitFactory
    SfxApplication* pApp = SFX_APP();
    if( pMatchP == &pApp->GetFilterMatcher() )
        pApp->ForcePendingInitFactories();
    if( nOrMask == 0xffff ) //Wg. Fehlbuild auf s
        nOrMask = 0;
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::Find_Impl()
{
    const SfxFilter* pFilter = 0;
    while( nCurrent < pMatch->pList->Count() )
    {
        pFilter = pMatch->pList->GetObject(nCurrent++);
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if( ((nFlags & nOrMask) == nOrMask ) && !(nFlags & nAndMask ) )
            break;
    }

    return pFilter;
}

const SfxFilter* SfxFilterMatcherIter::First()
{
    nCurrent = 0;
    return Find_Impl();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::Next()
{
    return Find_Impl();
}

/*---------------------------------------------------------------
    helper to build own formated string from given stringlist by
    using given seperator
  ---------------------------------------------------------------*/
::rtl::OUString implc_convertStringlistToString( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& lList     ,
                                                 const sal_Unicode&                                        cSeperator,
                                                 const ::rtl::OUString&                                    sPrefix   )
{
    ::rtl::OUStringBuffer   sString ( 1000 )           ;
    sal_Int32               nCount  = lList.getLength();
    sal_Int32               nItem   = 0                ;
    for( nItem=0; nItem<nCount; ++nItem )
    {
        if( sPrefix.getLength() > 0 )
        {
            sString.append( sPrefix );
        }
        sString.append( lList[nItem] );
        if( nItem+1<nCount )
        {
            sString.append( cSeperator );
        }
    }
    return sString.makeStringAndClear();
}

void SfxFilterContainer::ReadFilters_Impl()
{
    RTL_LOGFILE_CONTEXT( aMeasure, "sfx2 (as96863) ::SfxFilterContainer::ReadFilters" );
    BOOL bFirstRead = FALSE;
    if ( !pFilterArr )
    {
        pFilterArr = new SfxFilterList_Impl;
        bFirstRead = TRUE;
    }

    SfxFilterList_Impl& rList = *pFilterArr;
     new SfxFilterListener();

    try
    {
        // get the FilterFactory service to access the registered filters ... and types!
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     xFilterCFG                                                ;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     xTypeCFG                                                  ;
        if( xServiceManager.is() == sal_True )
        {
            xFilterCFG = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );
            xTypeCFG   = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" ) ), ::com::sun::star::uno::UNO_QUERY );
        }

        if(
            ( xFilterCFG.is() == sal_True ) &&
            ( xTypeCFG.is()   == sal_True )
          )
        {
            // select right query to get right set of filters for search modul
            ::com::sun::star::uno::Sequence< ::rtl::OUString > lFilterNames = xFilterCFG->getElementNames();
            if ( lFilterNames.getLength() )
            {
                // If list of filters already exist ...
                // ReadExternalFilters must work in update mode.
                // Best way seams to mark all filters NOT_INSTALLED
                // and change it back for all valid filters afterwards.
                BOOL bUpdated = FALSE;
                if( rList.Count() > 0 )
                {
                    bUpdated = TRUE;
                    USHORT nCount = (USHORT)rList.Count();
                    SfxFilter* pFilter;
                    for (USHORT f=0; f<nCount; ++f)
                    {
                        pFilter = NULL;
                        pFilter = rList.GetObject(f);
                        pFilter->nFormatType |= SFX_FILTER_NOTINSTALLED;
                    }
                }

                // get all properties of filters ... put it into the filter container
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lFilterProperties                           ;
                sal_Int32                                                                 nFilterCount      = lFilterNames.getLength();
                sal_Int32                                                                 nFilter           = 0                       ;

                for( nFilter=0; nFilter<nFilterCount; ++nFilter )
                {
                    // Try to get filter .. but look for any exceptions!
                    // May be filter was deleted by another thread ...
                    ::rtl::OUString sFilterName = lFilterNames[nFilter];
                    ::com::sun::star::uno::Any aResult;
                    try
                    {
                        aResult = xFilterCFG->getByName( sFilterName );
                    }
                    catch( ::com::sun::star::container::NoSuchElementException& )
                    {
                        aResult = ::com::sun::star::uno::Any();
                    }

                    if( aResult >>= lFilterProperties )
                    {
                        // collect informations to add filter to container
                        // (attention: some informations aren't available on filter directly ... you must search for corresponding type too!)
                        sal_Int32       nFlags          = 0 ;
                        sal_Int32       nClipboardId    = 0 ;
                        sal_Int32       nDocumentIconId = 0 ;
                        sal_Int32       nFormatVersion  = 0 ;
                        ::rtl::OUString sMimeType           ;
                        ::rtl::OUString sType               ;
                        ::rtl::OUString sUIName             ;
                        ::rtl::OUString sHumanName          ;
                        ::rtl::OUString sDefaultTemplate    ;
                        ::rtl::OUString sUserData           ;
                        ::rtl::OUString sExtension          ;
                        ::rtl::OUString sServiceName;

                        // first get directly available properties
                        sal_Int32 nFilterPropertyCount = lFilterProperties.getLength();
                        sal_Int32 nFilterProperty      = 0                            ;
                        for( nFilterProperty=0; nFilterProperty<nFilterPropertyCount; ++nFilterProperty )
                        {
                            if( lFilterProperties[nFilterProperty].Name.compareToAscii( "FileFormatVersion" ) == 0 )
                            {
                                lFilterProperties[nFilterProperty].Value >>= nFormatVersion;
                            }
                            else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "TemplateName" ) == 0 )
                            {
                                lFilterProperties[nFilterProperty].Value >>= sDefaultTemplate;
                            }
                            else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "Flags" ) == 0 )
                            {
                                lFilterProperties[nFilterProperty].Value >>= nFlags;
                            }
                            else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "UIName" ) == 0 )
                            {
                                lFilterProperties[nFilterProperty].Value >>= sUIName;
                            }
                            else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "UserData" ) == 0 )
                            {
                                ::com::sun::star::uno::Sequence< ::rtl::OUString > lUserData;
                                lFilterProperties[nFilterProperty].Value >>= lUserData;
                                sUserData = implc_convertStringlistToString( lUserData, ',', ::rtl::OUString() );
                            }
                            else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "DocumentService" ) == 0 )
                            {
                                lFilterProperties[nFilterProperty].Value >>= sServiceName;
                            }
                            else if( lFilterProperties[nFilterProperty].Name.compareToAscii( "Type" ) == 0 )
                            {
                                lFilterProperties[nFilterProperty].Value >>= sType;
                                // Try to get filter .. but look for any exceptions!
                                // May be filter was deleted by another thread ...
                                try
                                {
                                    aResult = xTypeCFG->getByName( sType );
                                }
                                catch( ::com::sun::star::container::NoSuchElementException& )
                                {
                                    aResult = ::com::sun::star::uno::Any();
                                }

                                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lTypeProperties;
                                if( aResult >>= lTypeProperties )
                                {
                                    // get indirect available properties then (types)
                                    sal_Int32 nTypePropertyCount = lTypeProperties.getLength();
                                    sal_Int32 nTypeProperty      = 0                          ;
                                    for( nTypeProperty=0; nTypeProperty<nTypePropertyCount; ++nTypeProperty )
                                    {
                                        if( lTypeProperties[nTypeProperty].Name.compareToAscii( "ClipboardFormat" ) == 0 )
                                        {
                                            lTypeProperties[nTypeProperty].Value >>= sHumanName;
                                        }
                                        else if( lTypeProperties[nTypeProperty].Name.compareToAscii( "DocumentIconID" ) == 0 )
                                        {
                                            lTypeProperties[nTypeProperty].Value >>= nDocumentIconId;
                                        }
                                        else if( lTypeProperties[nTypeProperty].Name.compareToAscii( "MediaType" ) == 0 )
                                        {
                                            lTypeProperties[nTypeProperty].Value >>= sMimeType;
                                        }
                                        else if( lTypeProperties[nTypeProperty].Name.compareToAscii( "Extensions" ) == 0 )
                                        {
                                            ::com::sun::star::uno::Sequence< ::rtl::OUString > lExtensions;
                                            lTypeProperties[nTypeProperty].Value >>= lExtensions;
                                            sExtension = implc_convertStringlistToString( lExtensions, ';', DEFINE_CONST_UNICODE("*.") );
                                        }
                                        else if( lTypeProperties[nTypeProperty].Name.compareToAscii( "URLPattern" ) == 0 )
                                        {
                                            lTypeProperties[nTypeProperty].Value >>= nFormatVersion;
                                        }
                                    }
                                }
                            }
                        }

                        if ( !sServiceName.getLength() )
                            continue;

                        // old formats are found ... using HumanPresentableName!
                        if( sHumanName.getLength() )
                        {
                            nClipboardId = SotExchange::RegisterFormatName( sHumanName );

                            // #100570# For external filters ignore clipboard IDs
                            if((nFlags & SFX_FILTER_STARONEFILTER) == SFX_FILTER_STARONEFILTER)
                            {
                                nClipboardId = 0;
                            }
                        }
                        // register SfxFilter
                        // first erase module name from old filter names!
                        // e.g: "scalc: DIF" => "DIF"
                        sal_Int32 nStartRealName = sFilterName.indexOf( DEFINE_CONST_UNICODE(": "), 0 );
                        if( nStartRealName != -1 )
                        {
                            DBG_ERROR("Old format, not supported!");
                            sFilterName = sFilterName.copy( nStartRealName+2 );
                        }

                        USHORT nCachePos = USHRT_MAX;
                        if ( (nFlags & SFX_FILTER_DEFAULT) == SFX_FILTER_DEFAULT )
                            nCachePos = 0;

                        SfxFilter* pFilter = !bFirstRead ? (SfxFilter*) SfxFilter::GetFilterByName( sFilterName ) : 0;
                        BOOL bNew = FALSE;
                        if (!pFilter)
                        {
                            bNew = TRUE;
                            pFilter = new SfxFilter( sFilterName             ,
                                                     sExtension              ,
                                                     nFlags                  ,
                                                     nClipboardId            ,
                                                     sType                   ,
                                                     sType                   ,
                                                     (USHORT)nDocumentIconId ,
                                                     sMimeType               ,
                                                     NULL/*this*/            ,
                                                     sUserData               ,
                                                     sServiceName );
                        }
                        else
                        {
                            pFilter->aName        = sFilterName;
                            pFilter->aWildCard    = WildCard(sExtension, ';');
                            pFilter->nFormatType  = nFlags;
                            pFilter->lFormat      = nClipboardId;
                            pFilter->aTypeName    = sType;
                            pFilter->nDocIcon     = (USHORT)nDocumentIconId;
                            pFilter->aMimeType    = sMimeType;
                            pFilter->aUserData    = sUserData;
                            pFilter->aServiceName = sServiceName;
                        }

                        // Don't forget to set right UIName!
                        // Otherwise internal name is used as fallback ...
                        pFilter->SetUIName( sUIName );
                        pFilter->SetDefaultTemplate( sDefaultTemplate );
                        if( nFormatVersion )
                        {
                            pFilter->SetVersion( nFormatVersion );
                        }

                        if (bNew)
                            rList.Insert( pFilter, nCachePos );
                    }
                }

                // In case we updated an already filled cache, it was to complicated to
                // look for right place of the default filter!
                // It seams to be easiear to step over the whole container twice and
                // correct it now ...
                if (bUpdated)
                {
                    SfxFilter*          pOldDefault = rList.First();
                    SfxFilter*          pNewDefault = NULL         ;
                    sal_Int32           nNewPos     = 0            ;

                    if ((pOldDefault->nFormatType & SFX_FILTER_DEFAULT) != SFX_FILTER_DEFAULT)
                    {
                        USHORT nCount = (USHORT)rList.Count();
                        SfxFilter* pFilter;
                        for (USHORT f=0; f<nCount; ++f)
                        {
                            pFilter = NULL;
                            pFilter = rList.GetObject(f);
                            if ((pFilter->nFormatType & SFX_FILTER_DEFAULT) == SFX_FILTER_DEFAULT)
                            {
                                pNewDefault = pFilter;
                                nNewPos     = f;
                                break;
                            }
                        }

                        if (nNewPos>0 && pNewDefault)
                        {
                            rList.Remove( pNewDefault                 );
                            rList.Remove( pOldDefault                 );
                            rList.Insert( pNewDefault, (ULONG)0       );
                            rList.Insert( pOldDefault, (ULONG)nNewPos );
                        }
                    }
                }
            }
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_ASSERT( sal_False, "SfxFilterContainer::ReadFilter()\nException detected. Possible not all filters could be cached.\n" );
    }

    if ( pContArr )
    {
        for ( USHORT n=0; n<pContArr->Count(); n++ )
            pContArr->GetObject(n)->Update();
    }
}

sal_Bool SfxFilterContainer_Impl::equalFilterNames( const String& s1, const ::rtl::OUString& s2 ) const
{
    ::rtl::OUString sC1( s1 );
    ::rtl::OUString sC2( s2 );
    sal_Int32       nPos = -1;

    nPos = sC1.indexOf( DEFINE_CONST_UNICODE(": "), 0 );
    if( nPos > 0 )
    {
        sC1 = sC1.copy( nPos+2 );
    }

    nPos = sC2.indexOf( DEFINE_CONST_UNICODE(": "), 0 );
    if( nPos > 0 )
    {
        sC2 = sC2.copy( nPos+2 );
    }

    return( sC1 == sC2 );
}

const SfxFilter* SfxFilterContainer::GetDefaultFilter_Impl( const String& rName )
{
    if ( !pFilterArr )
        ReadFilters_Impl();

    String aName( rName );
    USHORT nIndex = aName.SearchAscii(": ");
    if (  nIndex != STRING_NOTFOUND )
    {
        DBG_ERROR("Old filter name used!");
        aName = rName.Copy( nIndex + 2 );
    }

    const SfxFilter* pFirstFilter=0;
    sal_uInt16 nCount = ( sal_uInt16 ) pFilterArr->Count();
    for( sal_uInt16 n = 0; n < nCount; n++ )
    {
        const SfxFilter* pFilter = pFilterArr->GetObject( n );
        if ( pFilter->GetServiceName().CompareIgnoreCaseToAscii( aName ) == COMPARE_EQUAL )
        {
            if ( pFilter->GetFilterFlags() & SFX_FILTER_PREFERED )
                return pFilter;
            else if ( !pFirstFilter )
                pFirstFilter = pFilter;
        }
    }

    return pFirstFilter;
}
