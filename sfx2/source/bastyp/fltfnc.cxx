/*************************************************************************
 *
 *  $RCSfile: fltfnc.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: mba $ $Date: 2001-03-16 13:41:29 $
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

#ifndef _EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
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
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
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
#ifndef _INET_CONFIG_HXX //autogen
#include <inet/inetcfg.hxx>
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
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINMODE_HPP_
#include <com/sun/star/plugin/PluginMode.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINDESCRIPTION_HPP_
#include <com/sun/star/plugin/PluginDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINEXCEPTION_HPP_
#include <com/sun/star/plugin/PluginException.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINVARIABLE_HPP_
#include <com/sun/star/plugin/PluginVariable.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGIN_HPP_
#include <com/sun/star/plugin/XPlugin.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINMANAGER_HPP_
#include <com/sun/star/plugin/XPluginManager.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINCONTEXT_HPP_
#include <com/sun/star/plugin/XPluginContext.hpp>
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

#include <sal/types.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <rtl/ustring.hxx>
#include <vos/process.hxx>
#include <svtools/pathoptions.hxx>

#ifndef _L2TXTENC_HXX
#include <tools/l2txtenc.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;
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
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#include "app.hxx"
#include "fltdlg.hxx"
#include "sfxhelp.hxx"
#include "sfxbasic.hxx"
#include "docfilt.hxx"
#include "docfac.hxx"
#include "sfxtypes.hxx"
#include "sfxuno.hxx"
#include "docfile.hxx"
#include "progress.hxx"
#include "loadenv.hxx"
#include "iodlg.hxx"
#include "openflag.hxx"
#include "bastyp.hrc"
#include "sfxresid.hxx"
#include "doctempl.hxx"
#include "frame.hxx"
#include "dispatch.hxx"
#include "urlframe.hxx"
#include "picklist.hxx"
#include "topfrm.hxx"
#include "plugwin.hxx"
#include "helper.hxx"

// wg. EXPLORER_BROWSER
#include "request.hxx"
#include "nfltdlg.hxx"
#include "arrdecl.hxx"
#include "appdata.hxx"
#include "appuno.hxx"
#include "viewfrm.hxx"
#include "ucbhelp.hxx"

#define SFX_STR_OVERRIDE "Override"

#ifndef ERRCODE_SFX_RESTART
#define ERRCODE_SFX_RESTART 0
#endif

struct FlagMapping_Impl
{
    const char* pName;
    SfxFilterFlags nValue;
};

#ifdef TF_FILTER//MUSTFILTER

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

#ifndef __SGI_STL_ITERATOR
#include <iterator>
#endif

// Hash code function for calculation of map key.
struct TStringHashFunction
{
    size_t operator()(const ::rtl::OUString& sString) const
    {
        return sString.hashCode();
    }
};

// Declaration of string hash table for substitution of filter names.
typedef ::std::hash_map<    ::rtl::OUString                     ,
                            ::rtl::OUString                     ,
                            TStringHashFunction                 ,
                            ::std::equal_to< ::rtl::OUString >  >   TFilterNames;

typedef TFilterNames::const_iterator    TConstConverterIterator;

TFilterNames            aConverterOld2New;
TFilterNames            aConverterNew2Old;
TFilterNames            aUserDataTable;

//*****************************************************************************************************************
// Fill hash to convert old filter name to new filter names.
void impl_initUserDataTable( TFilterNames& aHash )
{
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarOffice XML (Writer)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CXML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW5V"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW4"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW4V"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW3V"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 2.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("SWG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 1.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("SWG1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter DOS"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("SW6"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HTML (StarWriter)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("HTML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXTA"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Unix"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXTX"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Mac"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXTM"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text DOS"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXTD"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rich Text Format"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("RTF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CWW6"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97 Vorlage"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CWW8"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95 Vorlage"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CWW6"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 6.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CWW6"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 6.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W05_3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 5"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("WW6"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 2.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W44_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 1.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W44_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 5.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W05_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 4.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W05_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 3.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W05_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 5.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W54_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 4.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W54_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 3.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W54_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 1 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W59_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 2 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W60_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 3 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W60_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 5.1-5.2 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W07_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W48_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.1 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W48_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 7.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W48_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.1 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W06_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.2 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W06_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W07_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.1 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W07_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W48_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.1 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W48_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar (Win) 1.x-2.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W37_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W09_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.5 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W09_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.3x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W04_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.45 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W04_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 4.0  (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W04_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.0  (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W04_3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.5  (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W04_4"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 6.0  (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W04_5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 7.0  (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W04_6"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 1.1-1.2 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W33_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 2.0-3.1 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W33_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 4.0 (StarWriter)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("EXCEL"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 5.0 (StarWriter)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CEXCEL"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 95 (StarWriter)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CEXCEL"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 2.0 DOS (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W39_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 3.0 Win (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W39_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 4.0 Mac (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W58_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write 4.x 5.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W51_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write II (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W52_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write Pro (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W56_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (DOS) (StarWriter)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("LOTUSD"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (WIN) (StarWriter)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("LOTUSW"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus Manuscript (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W24_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.0-8.3 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W31_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.5-9.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W31_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Claris Works (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W57_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: CTOS DEF (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W36_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 4.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W16_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 5.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W16_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 6.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W16_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III ( W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W17_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III+ ( W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W17_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Signature (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W17_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Sig. (Win) (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W17_3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite IV (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W17_4"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite (Win) 1.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W17_5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W103_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (Illustrator) (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W103_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Color Bitmap) (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W103_4"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Res Graphic) (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W103_8"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WriteNow 3.0 (Macintosh) (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W62_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Writing Assistant (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W13_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter Deluxe (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W11_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter 3 and 4 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W14_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 3.3 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W10_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. 3.6 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W10_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. II 3.7 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W10_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 4 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W10_3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: NAVY DIF (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W18_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS Write (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W08_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 1.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W08_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 2.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W08_3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 3.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W08_5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 1.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W08_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 2.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W08_4"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write Plus (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W33_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Peach Text (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W27_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA Revisable Form Text (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W15_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA with Display Write 5 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W15_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA/FFT-Final Form Text (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W32_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC DX (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W30_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC WPS-PLUS (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W45_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 2.0-4.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W15_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 5.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W15_3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DataGeneral CEO Write (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W104_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: EBCDIC (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W02_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Enable (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W28_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 3.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W42_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 4.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W42_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 5.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W42_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work III (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W29_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work IV  (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W29_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HP AdvanceWrite Plus (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W22_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 6 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W102_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 7 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W102_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W35_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf 5 - 6 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W46_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Legacy Winstar onGO (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W37_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 1.0-3.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W23_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 4.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W23_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W25_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.2 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W25_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Samna Word IV-IV Plus (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W22_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Total Word (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W14_1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex onGO (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W37_2"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex V7-V8 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W101_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang PC (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W26_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang II SWP (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W88_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang WP Plus (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W89_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Win Write 3.x (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W43_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WITA (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W34_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WiziWord 3.0 (W4W)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("W4W47_0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text (encoded)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXT_DLG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: HTML"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("HTML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 5.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW5VWEB"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 4.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW4VWEB"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (StarWriter/Web)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXTA"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text DOS (StarWriter/Web)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXTD"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Mac (StarWriter/Web)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXTM"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Unix (StarWriter/Web)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXTX"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 5.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 4.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW4"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 3.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (encoded)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXT_DLG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0/GlobalDocument"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0/GlobalDocument"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW4"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW4"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 3.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("CSW3"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: Text (encoded)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("TEXT_DLG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarOffice XML (Impress)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("XML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 Vorlage"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0 Vorlage"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 (StarImpress)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 Vorlage (StarImpress)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 (StarImpress)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 Vorlage (StarImpress)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("emp???**.dll"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97 Vorlage"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("emp???**.dll"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: CGM - Computer Graphics Metafile"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("icg???**.dll"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 (packed)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarOffice XML (Draw)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("XML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: GIF - Graphics Interchange"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCD - Photo CD"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCX - Zsoft Paintbrush"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PSD - Adobe Photoshop"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PNG - Portable Network Graphic"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PBM - Portable Bitmap"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PGM - Portable Graymap"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PPM - Portable Pixelmap"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: RAS - Sun Rasterfile"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TGA - Truevision TARGA"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGV - StarDraw 2.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TIF - Tag Image File"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGF - StarOffice Writer SGF"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XPM"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0 Vorlage"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 (StarDraw)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 Vorlage (StarDraw)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 (StarDraw)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 Vorlage (StarDraw)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0 Vorlage"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EMF - MS Windows Metafile"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: MET - OS/2 Metafile"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: DXF - AutoCAD Interchange"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EPS - Encapsulated PostScript"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: WMF - MS Windows Metafile"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCT - Mac Pict"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SVM - StarView Metafile"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: BMP - MS Windows"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: JPG - JPEG"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XBM - X-Consortium"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarOffice XML (Calc)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0 Vorlage/Template"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Rich Text Format (StarCalc)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: SYLK"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: DIF"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: HTML (StarCalc)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: dBase"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Lotus"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 1.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Text - txt - csv (StarCalc)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarOffice XML (Chart)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("XML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 5.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 4.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 3.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarOffice XML (Math)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathML XML (Math)"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 5.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 4.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 3.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 2.0"))] = OUString(RTL_CONSTASCII_USTRINGPARAM("1"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathType 3.x"))] = OUString(RTL_CONSTASCII_USTRINGPARAM(""));
}



// Fill hash to convert old filter name to new filter names.
void impl_initFilterHashOld2New( TFilterNames& aHash )
{
    // key = old filter name, value = new name
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarOffice XML (Writer)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarOffice_XML_Writer"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 2.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_20"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 1.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_10"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter DOS"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_DOS"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HTML (StarWriter)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HTML_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Unix"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Unix"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Mac"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Mac"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text DOS"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_DOS"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rich Text Format"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rich_Text_Format"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97 Vorlage"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95 Vorlage"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 6.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_60"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 6.x (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_6x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 5"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 2.x (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_2x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 5.x (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_5x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.1 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_61_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 7.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_70_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar (Win) 1.x-2.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_Win_1x_20_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 7.0  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_70_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 1.1-1.2 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_11_12_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 2.0-3.1 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_20_31_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 4.0 (StarWriter)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_40_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 5.0 (StarWriter)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_50_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 95 (StarWriter)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_95_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 2.0 DOS (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_20_DOS_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 3.0 Win (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_30_Win_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (DOS) (StarWriter)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_DOS_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (WIN) (StarWriter)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_WIN_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 5.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Win Write 3.x (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Win_Write_3x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text (encoded)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_encoded"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 1.x (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_1x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 5.x (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_5x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 4.x (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_4x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 3.x (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_3x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 4.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 3.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 1 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_1_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 2 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_2_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 3 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_3_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 5.1-5.2 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_51_52_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_60_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.1 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_41_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.2 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_42_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.1 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_51_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_60_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.1 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_61_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.5 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_35_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.3x (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_33x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.45 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_345_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 4.0  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.0  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.5  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_55_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 6.0  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_60_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 4.0 Mac (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_40_Mac_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write 4.x 5.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_4x_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write II (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_II_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write Pro (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_Pro_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus Manuscript (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_Manuscript_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.0-8.3 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_80_83_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.5-9.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_85_90_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Claris Works (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Claris_Works_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: CTOS DEF (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_CTOS_DEF_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 4.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 5.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 6.x (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_6x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III ( W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_III_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III+ ( W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_IIIP_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Signature (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Signature_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Sig. (Win) (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Sig_Win_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite IV (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_IV_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite (Win) 1.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Win_10_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (Illustrator) (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_Illustrator_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Color Bitmap) (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Color_Bitmap_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Res Graphic) (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Res_Graphic_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WriteNow 3.0 (Macintosh) (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WriteNow_30_Macintosh_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Writing Assistant (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Writing_Assistant_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter Deluxe (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_Deluxe_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter 3 and 4 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_3_and_4_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 3.3 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_33_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. 3.6 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_36_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. II 3.7 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_II_37_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 4 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_4_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: NAVY DIF (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_NAVY_DIF_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS Write (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_Write_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 1.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_10_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 2.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_20_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 3.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 1.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_10_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 2.x (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_2x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write Plus (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_Plus_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Peach Text (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Peach_Text_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA Revisable Form Text (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_Revisable_Form_Text_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA with Display Write 5 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_with_Display_Write_5_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA/FFT-Final Form Text (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCAFFT_Final_Form_Text_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC DX (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_DX_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC WPS-PLUS (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_WPS_PLUS_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 2.0-4.x (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_20_4x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 5.x (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_5x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DataGeneral CEO Write (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DataGeneral_CEO_Write_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: EBCDIC (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_EBCDIC_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Enable (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Enable_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 3.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 4.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work III (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_III_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work IV  (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_IV_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HP AdvanceWrite Plus (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HP_AdvanceWrite_Plus_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 6 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_6_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 7 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_7_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf 5 - 6 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_5_6_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Legacy Winstar onGO (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Legacy_Winstar_onGO_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 1.0-3.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_10_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 4.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_10_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.2 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_12_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Samna Word IV-IV Plus (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Samna_Word_IV_IV_Plus_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Total Word (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Total_Word_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex onGO (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_onGO_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex V7-V8 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_V7_V8_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang PC (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_PC_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang II SWP (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_II_SWP_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang WP Plus (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_WP_Plus_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WITA (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WITA_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WiziWord 3.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WiziWord_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: HTML"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_HTML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 5.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_50_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 4.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_40_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (StarWriter/Web)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_StarWriterWeb"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text DOS (StarWriter/Web)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_DOS_StarWriterWeb"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Mac (StarWriter/Web)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Mac_StarWriterWeb"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Unix (StarWriter/Web)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Unix_StarWriterWeb"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (encoded)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_encoded"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarOffice XML (GlobalDocument)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_60GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0/GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0/GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 3.0"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: Text (encoded)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_Text_encoded"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarOffice XML (Chart)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarOffice_XML_Chart"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 5.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 4.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 3.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarOffice XML (Calc)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarOffice_XML_Calc"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97 Vorlage/Template"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95 Vorlage/Template"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95 Vorlage/Template"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Rich Text Format (StarCalc)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Rich_Text_Format_StarCalc"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: SYLK"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_SYLK"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: DIF"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_DIF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: HTML (StarCalc)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_HTML_StarCalc"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: dBase"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_dBase"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Lotus"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Lotus"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 1.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_10"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Text - txt - csv (StarCalc)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Text_txt_csv_StarCalc"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarOffice XML (Impress)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarOffice_XML_Impress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 Vorlage"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0 Vorlage"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 (StarImpress)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_StarImpress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 Vorlage (StarImpress)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_Vorlage_StarImpress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 (StarImpress)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_StarImpress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 Vorlage (StarImpress)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_Vorlage_StarImpress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97 Vorlage"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: CGM - Computer Graphics Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_CGM_Computer_Graphics_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 (packed)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_packed"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarOffice XML (Draw)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarOffice_XML_Draw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: GIF - Graphics Interchange"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_GIF_Graphics_Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCD - Photo CD"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCD_Photo_CD"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCX - Zsoft Paintbrush"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCX_Zsoft_Paintbrush"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PSD - Adobe Photoshop"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PSD_Adobe_Photoshop"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PNG - Portable Network Graphic"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PNG_Portable_Network_Graphic"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PBM - Portable Bitmap"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PBM_Portable_Bitmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PGM - Portable Graymap"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PGM_Portable_Graymap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PPM - Portable Pixelmap"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PPM_Portable_Pixelmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: RAS - Sun Rasterfile"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_RAS_Sun_Rasterfile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TGA - Truevision TARGA"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TGA_Truevision_TARGA"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGV - StarDraw 2.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGV_StarDraw_20"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TIF - Tag Image File"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TIF_Tag_Image_File"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGF - StarOffice Writer SGF"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGF_StarOffice_Writer_SGF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XPM"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XPM"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0 Vorlage"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 (StarDraw)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_StarDraw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 Vorlage (StarDraw)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_Vorlage_StarDraw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 (StarDraw)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_StarDraw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 Vorlage (StarDraw)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_Vorlage_StarDraw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0 Vorlage"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EMF - MS Windows Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EMF_MS_Windows_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: MET - OS/2 Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_MET_OS2_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: DXF - AutoCAD Interchange"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_DXF_AutoCAD_Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EPS - Encapsulated PostScript"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EPS_Encapsulated_PostScript"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: WMF - MS Windows Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_WMF_MS_Windows_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCT - Mac Pict"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCT_Mac_Pict"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SVM - StarView Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SVM_StarView_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: BMP - MS Windows"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_BMP_MS_Windows"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: JPG - JPEG"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_JPG_JPEG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XBM - X-Consortium"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XBM_X_Consortium"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathML XML (Math)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathML_XML_Math"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarOffice XML (Math)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarOffice_XML_Math"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 2.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_20"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathType 3.x"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathType_3x"));
}

//*****************************************************************************************************************
// Fill hash to convert new filter or type name to old filter names
void impl_initFilterHashNew2Old( TFilterNames& aHash )
{
    // key = new filter name, value = old name
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarOffice_XML_Writer"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarOffice XML (Writer)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_20"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 2.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_10"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 1.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_DOS"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter DOS"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HTML_StarWriter"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HTML (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Unix"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Unix"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Mac"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Mac"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_DOS"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text DOS"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rich_Text_Format"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rich Text Format"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97_Vorlage"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95_Vorlage"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_60"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 6.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_6x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 6.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_5"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_2x_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 2.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_5x_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 5.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_61_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_70_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 7.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_Win_1x_20_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar (Win) 1.x-2.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_70_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 7.0  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_11_12_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 1.1-1.2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_20_31_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 2.0-3.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_40_StarWriter"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 4.0 (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_50_StarWriter"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 5.0 (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_95_StarWriter"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 95 (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_20_DOS_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 2.0 DOS (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_30_Win_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 3.0 Win (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_DOS_StarWriter"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (DOS) (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_WIN_StarWriter"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (WIN) (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_50_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Win_Write_3x_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Win Write 3.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_encoded"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text (encoded)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_1x_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 1.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_5x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 5.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_4x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 4.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_3x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 3.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_40_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 4.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_30_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_1_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_2_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_3_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 3 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_51_52_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 5.1-5.2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_60_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_41_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_42_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_50_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_51_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_60_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_61_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_30_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_35_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.5 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_33x_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.3x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_345_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.45 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_40_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 4.0  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_50_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.0  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_55_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.5  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_60_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 6.0  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_40_Mac_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 4.0 Mac (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_4x_50_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write 4.x 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_II_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write II (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_Pro_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write Pro (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_Manuscript_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus Manuscript (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_80_83_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.0-8.3 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_85_90_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.5-9.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Claris_Works_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Claris Works (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_CTOS_DEF_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: CTOS DEF (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_40_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 4.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_50_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_6x_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 6.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_III_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III ( W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_IIIP_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III+ ( W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Signature_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Signature (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Sig_Win_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Sig. (Win) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_IV_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite IV (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Win_10_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite (Win) 1.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_Illustrator_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (Illustrator) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Color_Bitmap_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Color Bitmap) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Res_Graphic_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Res Graphic) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WriteNow_30_Macintosh_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WriteNow 3.0 (Macintosh) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Writing_Assistant_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Writing Assistant (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_Deluxe_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter Deluxe (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_3_and_4_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter 3 and 4 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_33_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 3.3 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_36_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. 3.6 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_II_37_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. II 3.7 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_4_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 4 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_NAVY_DIF_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: NAVY DIF (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_Write_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS Write (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_10_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 1.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_20_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 2.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_30_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_10_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 1.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_2x_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 2.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_Plus_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write Plus (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Peach_Text_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Peach Text (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_Revisable_Form_Text_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA Revisable Form Text (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_with_Display_Write_5_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA with Display Write 5 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCAFFT_Final_Form_Text_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA/FFT-Final Form Text (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_DX_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC DX (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_WPS_PLUS_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC WPS-PLUS (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_20_4x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 2.0-4.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_5x_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 5.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DataGeneral_CEO_Write_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DataGeneral CEO Write (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_EBCDIC_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: EBCDIC (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Enable_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Enable (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_30_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_40_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 4.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_III_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work III (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_IV_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work IV  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HP_AdvanceWrite_Plus_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HP AdvanceWrite Plus (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_6_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 6 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_7_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 7 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_5_6_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf 5 - 6 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Legacy_Winstar_onGO_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Legacy Winstar onGO (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_10_30_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 1.0-3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_40_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 4.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_10_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_12_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Samna_Word_IV_IV_Plus_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Samna Word IV-IV Plus (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Total_Word_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Total Word (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_onGO_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex onGO (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_V7_V8_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex V7-V8 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_PC_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang PC (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_II_SWP_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang II SWP (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_WP_Plus_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang WP Plus (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WITA_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WITA (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WiziWord_30_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WiziWord 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_HTML"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: HTML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_50_VorlageTemplate"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 5.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_40_VorlageTemplate"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 4.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_StarWriterWeb"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (StarWriter/Web)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_DOS_StarWriterWeb"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text DOS (StarWriter/Web)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Mac_StarWriterWeb"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Mac (StarWriter/Web)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Unix_StarWriterWeb"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Unix (StarWriter/Web)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_encoded"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (encoded)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_60GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarOffice XML (GlobalDocument)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0/GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0/GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_30"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_Text_encoded"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: Text (encoded)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarOffice_XML_Chart"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarOffice XML (Chart)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_50"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_40"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_30"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarOffice_XML_Calc"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarOffice XML (Calc)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095_VorlageTemplate"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Rich_Text_Format_StarCalc"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Rich Text Format (StarCalc)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_SYLK"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: SYLK"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_DIF"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: DIF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_HTML_StarCalc"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: HTML (StarCalc)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_dBase"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: dBase"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Lotus"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Lotus"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_10"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 1.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Text_txt_csv_StarCalc"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Text - txt - csv (StarCalc)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarOffice_XML_Impress"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarOffice XML (Impress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_Vorlage"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40_Vorlage"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_StarImpress"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 (StarImpress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_Vorlage_StarImpress"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 Vorlage (StarImpress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_StarImpress"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 (StarImpress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_Vorlage_StarImpress"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 Vorlage (StarImpress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97_Vorlage"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_CGM_Computer_Graphics_Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: CGM - Computer Graphics Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_packed"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 (packed)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarOffice_XML_Draw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarOffice XML (Draw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_GIF_Graphics_Interchange"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: GIF - Graphics Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCD_Photo_CD"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCD - Photo CD"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCX_Zsoft_Paintbrush"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCX - Zsoft Paintbrush"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PSD_Adobe_Photoshop"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PSD - Adobe Photoshop"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PNG_Portable_Network_Graphic"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PNG - Portable Network Graphic"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PBM_Portable_Bitmap"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PBM - Portable Bitmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PGM_Portable_Graymap"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PGM - Portable Graymap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PPM_Portable_Pixelmap"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PPM - Portable Pixelmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_RAS_Sun_Rasterfile"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: RAS - Sun Rasterfile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TGA_Truevision_TARGA"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TGA - Truevision TARGA"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGV_StarDraw_20"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGV - StarDraw 2.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TIF_Tag_Image_File"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TIF - Tag Image File"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGF_StarOffice_Writer_SGF"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGF - StarOffice Writer SGF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XPM"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XPM"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("gif_Graphics_Interchange"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: GIF - Graphics Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("pcd_Photo_CD"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCD - Photo CD"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("pcx_Zsoft_Paintbrush"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCX - Zsoft Paintbrush"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("psd_Adobe_Photoshop"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PSD - Adobe Photoshop"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("png_Portable_Network_Graphic"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PNG - Portable Network Graphic"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("pbm_Portable_Bitmap"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PBM - Portable Bitmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("pgm_Portable_Graymap"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PGM - Portable Graymap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("ppm_Portable_Pixelmap"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PPM - Portable Pixelmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("ras_Sun_Rasterfile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: RAS - Sun Rasterfile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("tga_Truevision_TARGA"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TGA - Truevision TARGA"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sgv_StarDraw_20"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGV - StarDraw 2.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("tif_Tag_Image_File"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TIF - Tag Image File"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sgf_StarOffice_Writer_SGF"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGF - StarOffice Writer SGF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("xpm_XPM"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XPM"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50_Vorlage"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_StarDraw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 (StarDraw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_Vorlage_StarDraw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 Vorlage (StarDraw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_StarDraw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 (StarDraw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_Vorlage_StarDraw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 Vorlage (StarDraw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30_Vorlage"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EMF_MS_Windows_Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EMF - MS Windows Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_MET_OS2_Metafile"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: MET - OS/2 Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_DXF_AutoCAD_Interchange"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: DXF - AutoCAD Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EPS_Encapsulated_PostScript"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EPS - Encapsulated PostScript"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_WMF_MS_Windows_Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: WMF - MS Windows Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCT_Mac_Pict"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCT - Mac Pict"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SVM_StarView_Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SVM - StarView Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_BMP_MS_Windows"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: BMP - MS Windows"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_JPG_JPEG"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: JPG - JPEG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XBM_X_Consortium"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XBM - X-Consortium"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("emf_MS_Windows_Metafile"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EMF - MS Windows Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("met_OS2_Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: MET - OS/2 Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("dxf_AutoCAD_Interchange"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: DXF - AutoCAD Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("eps_Encapsulated_PostScript"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EPS - Encapsulated PostScript"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("wmf_MS_Windows_Metafile"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: WMF - MS Windows Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("pct_Mac_Pict"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCT - Mac Pict"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("svm_StarView_Metafile"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SVM - StarView Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("bmp_MS_Windows"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: BMP - MS Windows"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("jpg_JPEG"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: JPG - JPEG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("xbm_X_Consortium"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XBM - X-Consortium"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarOffice_XML_Math"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarOffice XML (Math)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathML_XML_Math"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathML XML (Math)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_20"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 2.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathType_3x"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathType 3.x"));
}


::rtl::OUString impl_getUserData( const ::rtl::OUString& sOldName )
{
    TConstConverterIterator pEntry = aUserDataTable.find(sOldName);
    ::rtl::OUString sUserData;
    if( pEntry!=aUserDataTable.end() )
        sUserData = aUserDataTable[sOldName];
    return sUserData;
}


//*****************************************************************************************************************
::rtl::OUString impl_getNewFilterName( const ::rtl::OUString& sOldName )
{
    // Search for existing entry! Don't use index operato directly - he create a new entry if it not already exist automaticly!
    TConstConverterIterator pEntry = aConverterOld2New.find(sOldName);
    // Warn programmer if some filter names are not suported yet!
//    DBG_ASSERT( !(pEntry==aConverterOld2New.end()), "SfxFrameLoader::impl_getNewFilterName()\nUnsupported filter name detected ... Convertion failed!\n" );
    ::rtl::OUString sNewName;
    if( pEntry!=aConverterOld2New.end() )
    {
        sNewName = aConverterOld2New[sOldName];
    }
    return sNewName;
}

//*****************************************************************************************************************
::rtl::OUString impl_getOldFilterName( const ::rtl::OUString& sNewName )
{
    // Search for existing entry! Don't use index operato directly - he create a new entry if it not already exist automaticly!
    TConstConverterIterator pEntry = aConverterNew2Old.find(sNewName);
    // Warn programmer if some filter names are not suported yet!
//    DBG_ASSERT( !(pEntry==aConverterNew2Old.end()), "SfxFrameLoader::impl_getOldFilterName()\nUnsupported filter name detected ... Convertion failed!\n" );
    ::rtl::OUString sOldName;
    if( pEntry!=aConverterNew2Old.end() )
    {
        sOldName = aConverterNew2Old[sNewName];
    }
    return sOldName;
}

#endif//MUSTFILTER

static const FlagMapping_Impl aMap[] =
{
    "Import",           SFX_FILTER_IMPORT,
    "Export",           SFX_FILTER_EXPORT,
    "Internal",         SFX_FILTER_INTERNAL,
    "Template",         SFX_FILTER_TEMPLATE,
    "TemplatePath",     SFX_FILTER_TEMPLATEPATH,
    "Own",              SFX_FILTER_OWN,
    "Alien",            SFX_FILTER_ALIEN,
    "Asynchron",        SFX_FILTER_ASYNC,
    "Readonly",         SFX_FILTER_OPENREADONLY,
    "UsesOptions",      SFX_FILTER_USESOPTIONS,
    "NotInstalled",     SFX_FILTER_MUSTINSTALL,
    "ConsultService",   SFX_FILTER_CONSULTSERVICE,
    "NotInChooser",     SFX_FILTER_NOTINCHOOSER,
    "NotInFileDialog",  SFX_FILTER_NOTINFILEDLG,
    "Packed",           SFX_FILTER_PACKED,
    "SilentExport",     SFX_FILTER_SILENTEXPORT,
    "Prefered",         SFX_FILTER_PREFERED,
    "BrowserPrefered",  SFX_FILTER_BROWSERPREFERED,
    0, 0
};

SfxFilterFlags NameToFlag_Impl( const String& aName )
/*   [Beschreibung]

     Konvertiert einen Namen aus der Install.ini in das zug. SFX_FILTER_FLAG

 */

{
    sal_uInt16 n = 0;
    for( const char* pc = aMap[0].pName; pc;
         pc = aMap[++n].pName)
        if( aName.EqualsAscii(pc) )
            return aMap[n].nValue;
    DBG_ERROR("Name nicht gefunden" );
    return 0;
}

String FlagsToName_Impl( SfxFilterFlags nValue )
/*   [Beschreibung]

     Konvertiert ein SFX_FILTER_FLAG in einen Text, der in die Install.ini
     geschrieben werden kann
 */
{
    ByteString aRet;
    sal_uInt16 n = 0;
    for( const char* pc = aMap[0].pName; pc;
         pc = aMap[++n].pName)
        if( nValue & aMap[n].nValue )
        {
            if( !aRet.Len() )
                aRet = pc;
            else
            {
                aRet+="|";
                aRet+=pc;
            }
        }

    return String(S2U(aRet));
}

//----------------------------------------------------------------

inline String ToUpper_Impl( const String &rStr )
{
    String aRet( rStr );
    CharClass aCharClass( Application::GetSettings().GetLocale() );
    aCharClass.toUpper( aRet );
    return aRet;
}

//----------------------------------------------------------------

struct LoadArg_Impl
{
    String aGroup;
    sal_Bool bInstallIni;
    SfxFilterFlags nOrFlags;
    SfxFilterFlags nNotFlags;
};

class SfxFilterContainer_Impl
{
public:
    SfxFilterContainer_Impl() : bLoadPending( sal_False ), pArg( 0 ) {}
    SfxFilterList_Impl aList;
    String aName;
    LoadArg_Impl* pArg;
    sal_Bool bLoadPending;
    SfxFilterContainerFlags eFlags;
};


//----------------------------------------------------------------

SfxFilterContainer::SfxFilterContainer( const String& rName )
{
    pImpl = new SfxFilterContainer_Impl;
    pImpl->eFlags = 0;
    pImpl->aName = rName;

    impl_initFilterHashOld2New( aConverterOld2New );
    impl_initFilterHashNew2Old( aConverterNew2Old );
    impl_initUserDataTable( aUserDataTable );
}

//----------------------------------------------------------------

SfxFilterContainer::~SfxFilterContainer()
{
    DELETEZ( pImpl->pArg );
    SfxFilterList_Impl& rList = pImpl->aList;
    sal_uInt16 nCount = (sal_uInt16 )rList.Count();
    for( sal_uInt16 n = 0; n<nCount; n++ )
        delete rList.GetObject( n );
    delete pImpl;
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

const SfxFilter* SfxFilterContainer::GetFilter4Protocol(
    SfxMedium& rMed, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*   [Beschreibung]

     Gibt den Filter zu einem bestimmten Protokoll zurueck. Diese Methode
     darf sich nicht auf Inhalte beziehen, sondern soll allein anhand von
     Protokoll / ::com::sun::star::util::URL einen Filter auswaehlen.
     In der Basisimplementierung werden einige Protokolle direkt auf
     GetFilter4FilterName abgebildet, man zur Integration des Filters
     in die Filterdetection einfach einen Filter des Protokollnames anlegen
     kann.
 */
{
    const INetURLObject& rObj = rMed.GetURLObject();
    String aName = rMed.GetURLObject().GetMainURL();
    // Hier noch pruefen, ob das Doc im Browser dargestellt werden soll
    SFX_ITEMSET_ARG( rMed.GetItemSet(), pTargetItem, SfxStringItem,
                     SID_TARGETNAME, sal_False);
    if( pTargetItem && pTargetItem->GetValue().EqualsAscii("_beamer") )
        return 0;
    return GetFilter4Protocol( aName, nMust, nDont );
}

const SfxFilter* SfxFilterContainer::GetFilter4Protocol( const String& rName, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    String aName( rName );
    ForceFilterLoad_Impl();
    aName.ToLowerAscii();
    sal_uInt16 nCount = ( sal_uInt16 ) pImpl->aList.Count();
    for( sal_uInt16 n = 0; n < nCount; n++ )
    {
        const SfxFilter* pFilter = pImpl->aList.GetObject( n );
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        String aString( pFilter->GetURLPattern());
        if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) &&
             WildCard(aString ).Matches( aName ) )
            return pFilter;
    }
    return 0;
}

//----------------------------------------------------------------

sal_uInt32 SfxFilterContainer::Execute( SfxMedium& rMedium, SfxFrame*& pFrame) const
{
    return ERRCODE_ABORT;
}

//----------------------------------------------------------------

sal_uInt16 SfxFilterContainer::GetFilterCount() const
{
    // Dazu muessen die Filter geladen werden
    ForceFilterLoad_Impl();
    return (sal_uInt16) pImpl->aList.Count();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterContainer::GetFilter( sal_uInt16 nPos ) const
{
    ForceFilterLoad_Impl();
    return pImpl->aList.GetObject( nPos );
}

IMPL_STATIC_LINK( SfxFilterContainer, LoadHdl_Impl, void*, EMPTYARG )
/*   [Beschreibung]

     Handler, der ueber LateInit das echte Laden des
     SfxFilterContainers aus;loesst.  */
{
    if( pThis->pImpl->bLoadPending )
    {
        pThis->pImpl->bLoadPending = sal_False;
//        pThis->RealLoad_Impl();
    }
    return 0;
}

//----------------------------------------------------------------

#include <stdio.h>

void SfxFilterContainer::RealLoad_Impl()
/*   [Beschreibung]

     Eigentliches Laden der Filter eines Containers aus der install.ini
 */

{
    static sal_Bool bRecurse = sal_False;
    rtl_TextEncoding aINIEncoding = RTL_TEXTENCODING_DONTKNOW;

    LoadArg_Impl* pArg = pImpl->pArg;
    String aString( pArg->aGroup);

    ByteString aFile( "f:\\filters.dat" );
    FILE* f = fopen( aFile.GetBuffer(), "a" );

    aString+=DEFINE_CONST_UNICODE("-Filters");
    if( pArg->bInstallIni )
    {
        if( !bRecurse )
        {
            bRecurse = sal_True;
            sal_uInt32 nCount = pImpl->aList.Count();
            RealLoad_Impl();
            bRecurse = sal_False;
            // Falls wir keine lokalisierten Filter gefunden haben, nehmen wir
            // die alten
            if( pImpl->aList.Count() != nCount )
            {
//              DELETEZ( pImpl->pArg );
                return;
            }
        }
        else
        {
            String sLang = String::CreateFromAscii(ResMgr::GetLang());
            aString += '-';
            aString += sLang;
            aINIEncoding = Langcode2TextEncoding((sal_uInt16)sLang.ToInt32());
        }
    }

    Config* pConfig = SFX_APP()->GetFilterIni();
    DBG_ASSERT( pConfig, "can not load the filter ini" );
    if( pArg->bInstallIni )
        pConfig->SetGroup( U2S(aString) );

    sal_uInt16 nCount = pArg->bInstallIni ? pConfig->GetKeyCount() : 0;
    String aOver( DEFINE_CONST_UNICODE(SFX_STR_OVERRIDE) );
    String aName, aLine, aUIType, aMimeType, aClipFormat, aMacType, aTypeName, aWild, aFlags, aDefaultTemplate, aUserData;
    for( sal_uInt16 n = 0; n < nCount; n++ )
    {
        aName = pArg->bInstallIni ? String(S2U(pConfig->GetKeyName( n ))) : String();
        aLine = pArg->bInstallIni ? String( pConfig->ReadKey( n ), aINIEncoding ) : String();
        sal_uInt16 nTokCount = aLine.GetTokenCount( ',' );
        if( nTokCount < 8 )
        {
#ifdef DBG_UTIL
            ByteString aMsg( "Falsches FilterFormat: " );
            aMsg += U2S(aLine).getStr();
            DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
            continue;
        }

#ifdef DBG_UTIL
        if( nTokCount < 10 )
        {
            ByteString aMsg( "Obsoletes FilterFormat: " );
            aMsg += U2S(aLine).getStr();
            DBG_ERRORFILE( aMsg.GetBuffer() );
        }
        static bWarned = sal_False;
        if( nTokCount > 11 && !bWarned )
        {
            bWarned = sal_True;
            ByteString aMsg( "Neueres FilterFormat: " );
            aMsg += U2S(aLine).getStr();
            DBG_ERRORFILE( aMsg.GetBuffer() );
        }
#endif
        // Override-Filter haben keinen Namen
        if( aName.Match( aOver ) >= aOver.Len() )
            aName.Erase();

        sal_uInt16 i = 0;
        if( nTokCount >= 10 )
            aUIType = aLine.GetToken( i++, ',' );
        else
            aUIType = aName;

        aMimeType = aLine.GetToken( i++, ',' );
        aClipFormat = aLine.GetToken( i++, ',' );
        aMacType = aLine.GetToken( i++, ',' );
        aTypeName = aLine.GetToken( i++, ',' );
        aWild = aLine.GetToken( i++, ',' );
        sal_uInt16 nDocIconId = (sal_uInt16) aLine.GetToken( i++, ',' ).ToInt32();
        aUserData = aLine.GetToken( i++, ',' );
        sal_uInt32 nVersion = SOFFICE_FILEFORMAT_50;
        if( nTokCount >= 8 )
            nVersion = aLine.GetToken( i++, ',' ).ToInt32();
        aFlags = aLine.GetToken( i++, ',' );
        if( nTokCount >= 11 )
            aDefaultTemplate = aLine.GetToken( i++, ',' );
        else aDefaultTemplate.Erase();
        SfxFilterFlags nFlags = pArg->nOrFlags;
        nTokCount = aFlags.GetTokenCount('|');
        for( i = 0; i < nTokCount; i++ )
        {
            String aTok = aFlags.GetToken( i, '|' );
            nFlags |= NameToFlag_Impl( aTok );
        }

        nFlags &= ~pArg->nNotFlags;
        sal_uInt32 nClipId = 0;
        if( aClipFormat.Len() )
        {
            ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
            aDataFlavor.MimeType = aClipFormat;
            nClipId = SotExchange::GetFormat( aDataFlavor );
        }

        SfxFilter* pFilter = new SfxFilter(
            aName, aWild, nFlags, nClipId, aMacType, aTypeName, nDocIconId,
            aMimeType, this, aUserData );
        pFilter->SetUIName( aUIType );
        pFilter->SetDefaultTemplate( aDefaultTemplate );
        if( nVersion )
            pFilter->SetVersion( nVersion );
//        AddFilter( pFilter, GetFilterCount() );

        String aLeft = String::CreateFromAscii("aHash[OUString(RTL_CONSTASCII_USTRINGPARAM(");
        aLeft += '"';
        aLeft += pFilter->GetName();
        aLeft += '"';
        aLeft += String::CreateFromAscii( "))] = OUString(RTL_CONSTASCII_USTRINGPARAM(" );
        aLeft += '"';
        aLeft += pFilter->GetUserData();
        aLeft += '"';
        aLeft += String::CreateFromAscii( "));" );
        ByteString aOut( aLeft, RTL_TEXTENCODING_UTF8 );
        fprintf( f, "%s\n", aOut.GetBuffer() );

        delete pFilter;
//    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarOffice XML (Writer)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarOffice_XML_Writer"));
    }

    fclose( f );
//    if( !bRecurse ) DELETEZ( pImpl->pArg );
}

//----------------------------------------------------------------

void SfxFilterContainer::ForceFilterLoad_Impl() const
/*   [Beschreibung]

     Erzwingt das echte Laden der Filter vor Eintritt des Lateinits
 */
{
    if( pImpl->bLoadPending )
    {
        ((SfxFilterContainer*)this)->LoadHdl_Impl(
            (SfxFilterContainer*)this ,0 );
    }
}

//----------------------------------------------------------------
/*   [Beschreibung]

     Da die meisten GetFilter4xxx Routinen in einem Container einfach eine
     Abfrage ueber alle enthaltenen Filter durchlaufen, sind diese in
     einem Makro kodiert.

     nMust sind die Flags, die gesetzt sein muessen, damit der Filter
     in Betracht gezogen wird, nDont duerfen nicht gesetzt sein.
 */

#define IMPL_CONTAINER_LOOP( aMethod, aArgType, aTest )         \
const SfxFilter* SfxFilterContainer::aMethod(                   \
    aArgType aArg, SfxFilterFlags nMust, SfxFilterFlags nDont ) const \
{                                                               \
    const SfxFilter* pFirstFilter=0;                            \
    ForceFilterLoad_Impl();                                     \
    sal_uInt16 nCount = ( sal_uInt16 ) pImpl->aList.Count();    \
    for( sal_uInt16 n = 0; n < nCount; n++ )                    \
    {                                                           \
        const SfxFilter* pFilter = pImpl->aList.GetObject( n ); \
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

/*   [Beschreibung]

     Ermitelt einen Filter nach seinem Namen. Dieser enthaelt vorangestellt
     den Namen des Containers, falls er nicht im DefaultContainer steckt.
 */

IMPL_CONTAINER_LOOP( GetFilter, const String&,
                     pFilter->GetName().CompareIgnoreCaseToAscii( aArg ) == COMPARE_EQUAL )

/*   [Beschreibung]
     Ermitelt einen Filter nach seinem Mimetypen.
 */
IMPL_CONTAINER_LOOP(
    GetFilter4Mime, const String&,
    pFilter->GetMimeType().CompareIgnoreCaseToAscii( aArg ) == COMPARE_EQUAL )

/*   [Beschreibung]
     Ermitelt einen Filter nach seinem FilterNamen. Dies ist der Name ohne
     vorangestellten Containernamen.
 */
IMPL_CONTAINER_LOOP(
    GetFilter4FilterName, const String&,
    pFilter->GetFilterName().CompareIgnoreCaseToAscii(aArg ) == COMPARE_EQUAL )

/*   [Beschreibung]
     Ermitelt einen Filter nach seiner ClipboardID. Diese steckt im Storage.
 */
IMPL_CONTAINER_LOOP(
    GetFilter4ClipBoardId, sal_uInt32, aArg && pFilter->GetFormat() == aArg )

#ifdef MAC
#define CONDITION \
    String( aArg ).Erase( 5 ) == String( pFilter->GetTypeName() ).Erase( 5 )
#else
#define CONDITION \
    aArg == pFilter->GetTypeName()
#endif
/*   [Beschreibung]
     Ermitelt einen Filter nach seinen Extended Attributes.
     Nur auf MAC und OS/2 von Interesse.
 */
IMPL_CONTAINER_LOOP(
    GetFilter4EA, const String&, CONDITION )
#undef CONDITION

/*   [Beschreibung]
     Ermitelt einen Filter nach seiner Extension.
     (2Ah = '*')
 */
IMPL_CONTAINER_LOOP(
    GetFilter4Extension, const String&,
    pFilter->GetWildcard() != String() && pFilter->GetWildcard() != DEFINE_CONST_UNICODE("*.*") && pFilter->GetWildcard() != 0x002A &&
    WildCard( ToUpper_Impl( pFilter->GetWildcard()() ), ';' ) == ToUpper_Impl( aArg ))

IMPL_CONTAINER_LOOP(
    GetFilter4UIName, const String&,
    pFilter->GetUIName() == aArg )

//----------------------------------------------------------------

const String SfxFilterContainer::GetName() const
{
    return pImpl->aName;
}

//----------------------------------------------------------------

void SfxFilterContainer::DeleteFilter( const SfxFilter* pFilter )
/*   [Beschreibung]

     Loescht einen Filter aus seinem Container und den Filter selbst.
 */
{
    ForceFilterLoad_Impl();
    pImpl->aList.Remove( (SfxFilter*)pFilter );
    delete (SfxFilter*)pFilter;
}

//----------------------------------------------------------------

void SfxFilterContainer::AddFilter( SfxFilter* pFilter, sal_uInt16 nPos )
{
/*   [Beschreibung]

     Fuegt einen Filter in einen Container ein.
 */
    ForceFilterLoad_Impl();
    if ( pImpl->pArg )
    {
        pFilter->nFormatType |= pImpl->pArg->nOrFlags;
        pFilter->nFormatType &= ~pImpl->pArg->nNotFlags;
    }

    if ( !pFilter->GetFilterName().Len() ||
         !GetFilter4FilterName( pFilter->GetFilterName() ))
        pImpl->aList.Insert( pFilter, nPos );
#if !defined(PRODUCT)
    else
        delete pFilter;
#endif
}

//----------------------------------------------------------------

void SfxFilterContainer::LoadFilters(
    const String& rGroup,  // ConfigGruppe, aus der gelesen werden soll
    sal_Bool bInstallIni,      // Falls sal_True aus install.ini lesen,
                           // sonst soffice3.ini
    SfxFilterFlags nOrFlags,       // Flags zusaetzlich zu denen in der Ini
    SfxFilterFlags nNotFlags )     // Flags, die nicht aus der ini genommen werden
/*   [Beschreibung]

     Stoesst das Laden eines FilterContainers an. Das eigentliche
     Laden erfolgt im LateInitHandler bzw. in ForceFilterLoad_Impl,
     falls vor LateInit auf den FilterContainer zugegriffen wird.  */
{
    LoadArg_Impl* pArg = new LoadArg_Impl;
    pArg->aGroup = rGroup;
    pArg->bInstallIni = bInstallIni;
    pArg->nOrFlags = nOrFlags;
    pArg->nNotFlags = nNotFlags;
    pImpl->pArg = pArg;
    SFX_APP()->InsertLateInitHdl( STATIC_LINK(
        this, SfxFilterContainer, LoadHdl_Impl ) );
    pImpl->bLoadPending = sal_True;
}

//----------------------------------------------------------------

void SfxFilterContainer::SaveFilters( const String& rGroup, SfxFilterFlags nMask ) const
/*   [Beschreibung]

     Speichert die Filter des Containers in der soffice3.ini.
 */
{
}

//-------------------------------------------------------------------------

sal_uInt32 SfxFilterContainer::GetFilter4Content(
    SfxMedium& rMedium, const SfxFilter** ppFilter,
    SfxFilterFlags, SfxFilterFlags ) const
{
    return 0;
}

//-------------------------------------------------------------------------
#if 0
sal_uInt32 SfxExecutableFilterContainer::Choose_Impl( SfxMedium& rMedium ) const
{
    SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
    SfxFilterDialog *pDlg =
        new SfxFilterDialog(
            0, &rMedium, rMatcher, 0, 0 );
    const sal_Bool bOk = RET_OK == pDlg->Execute();
    if (bOk)
    {
        const SfxFilter* pFilter  = rMatcher.GetFilter4UIName(
            pDlg->GetSelectEntry() );
        delete pDlg;
        rMedium.SetFilter( pFilter );
        return ERRCODE_NONE;
    }
    delete pDlg;
    return ERRCODE_ABORT;
}

String SfxExecutableFilterContainer::GetBugdocName_Impl(
    const String& rName ) const
/*   [Beschreibung]

     Sucht ein Bugdoc in den ueblichen Verzeichnissen
 */
{
    static const char* pNames[] =
    {
        "q:\\sw\\bugdoc",
        "q:\\sd\\bugdoc",
        "q:\\sc\\bugdoc",
        "q:\\sch\\bugdoc",
        "q:\\solar\\bugdoc",
        "q:\\bugdoc",
        0
    };
    sal_uInt32 nNumber = rName.ToInt32();
    String aMatch = rName;
    aMatch += '*';
    sal_uInt16 n = 1;
    const char* pName = pNames[ 0 ];
    while( pName )
    {
        DirEntry aEntry( String::CreateFromAscii(pName) );
        for( sal_uInt32 nBase = ( nNumber / 500 + 1 ) * 500;
             nBase - nNumber < 5000; nBase+=500 )
        {
            DirEntry aAkt( aEntry );
            String aBis( DEFINE_CONST_UNICODE("bis") );
            aBis += String::CreateFromInt32( nBase );
            aAkt += DirEntry( aBis );
            if( aAkt.Exists() )
            {
                aAkt += DirEntry( aMatch );
                Dir aDir( aAkt );
                if( aDir.Count() )
                    return aDir[ 0 ].GetFull();
            }
        }
        pName = pNames[ n++ ];
    }

    return String();
}

        case SFX_EXE_FILTER_BUGID:
        {
            String aPathName = DEFINE_CONST_UNICODE("http://webserver1.stardiv.de/Bugtracker/Source/Body_ReportDetail.asp?ID=");
            aPathName += rMedium.GetURLObject().GetURLPath();
            rMedium.SetName( aPathName );
            rMedium.SetPhysicalName( String() );
            rMedium.Init_Impl();
            rMedium.SetFilter( 0 );
            return ERRCODE_SFX_RESTART;
        }

        case SFX_EXE_FILTER_BUGDOC:
        {
            String aPathName = GetBugdocName_Impl(
                rMedium.GetURLObject().GetURLPath() );
            if( aPathName.Len() )
            {
                rMedium.SetName( aPathName );
                rMedium.SetPhysicalName( String() );
                rMedium.Init_Impl();
                rMedium.SetFilter( 0 );
                return ERRCODE_SFX_RESTART;
            }
            else return ERRCODE_SFX_INVALIDLINK;
        }

#endif

//----------------------------------------------------------------

SfxFactoryFilterContainer::SfxFactoryFilterContainer(
    const String& rName, const SfxObjectFactory& rFactP )
    : SfxFilterContainer( rName ), rFact( rFactP ), pFunc(0)
/*   [Beschreibung]

     Im SfxFactoryFilterContainer befinden sich die Filter einer
     SfxObjectFactory. Wird kein DetectFilter registriert, so wird
     fuer die DefaultFactory <SfxApplication::DetectFilter> genommen
 */
{
    //Defaults in den Container der DefaultFactory laden
    SetFlags( GetFlags() | SFX_FILTER_CONTAINER_FACTORY );
    if( &SfxObjectFactory::GetDefaultFactory() == &rFactP )
    {
        pFunc = &SfxFilterMatcher::AppDetectFilter;
    }
}
//----------------------------------------------------------------

sal_uInt32 SfxFactoryFilterContainer::GetFilter4Content(
    SfxMedium& rMedium, const SfxFilter** ppFilter, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    ForceFilterLoad_Impl();
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pTargetItem, SfxStringItem,
                     SID_TARGETNAME, sal_False);
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pDontItem, SfxBoolItem,
                     SID_FILTER_DONTDETECTONCONTENT, sal_False);
    if( pDontItem && pDontItem->GetValue() )
        return 0;

    if ( *ppFilter && ( (*ppFilter)->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
        return 0;

    if( pFunc )
    {
        sal_uInt32 nErr = (*pFunc)(rMedium, ppFilter, nMust, nDont);
        DBG_ASSERT( !*ppFilter ||
                    (((*ppFilter)->GetFilterFlags() & nMust ) == nMust &&
                     ((*ppFilter)->GetFilterFlags() & nDont ) == 0 ),
                    "DetectFilter Spec nicht eingehalten" );
        return nErr;
    }
    else
        return 0;
}

//----------------------------------------------------------------

DECLARE_LIST( SfxFContainerList_Impl, SfxFilterContainer * );

class SfxFilterMatcher_Impl
{
public:
    sal_uInt16 nAkt;
    SfxFContainerList_Impl aList;
    sal_Bool bDeleteContainers;
};

//----------------------------------------------------------------

SfxFilterMatcher::SfxFilterMatcher( SfxFilterContainer* pCont)
{
    pImpl = new SfxFilterMatcher_Impl;
    pImpl->bDeleteContainers = sal_False;
    AddContainer( pCont );
}

//----------------------------------------------------------------

SfxFilterMatcher::SfxFilterMatcher()
{
    pImpl = new SfxFilterMatcher_Impl;
    pImpl->bDeleteContainers = sal_False;
}

SfxFilterMatcher::SfxFilterMatcher(sal_Bool bDeleteContainers)
{
    pImpl = new SfxFilterMatcher_Impl;
    pImpl->bDeleteContainers = bDeleteContainers;
}

//----------------------------------------------------------------

SfxFilterMatcher::~SfxFilterMatcher()
{
    if ( pImpl->bDeleteContainers )
    {
        for ( sal_uInt32 n = pImpl->aList.Count(); n--; )
        {
            SfxFilterContainer *pFCont = pImpl->aList.Remove(n);
            delete pFCont;
        }
    }
    delete pImpl;
}

//----------------------------------------------------------------

void SfxFilterMatcher::AddContainer( SfxFilterContainer* pC )
{
    pImpl->aList.Insert( pC, pImpl->aList.Count() );
}

//----------------------------------------------------------------

sal_uInt16 SfxFilterMatcher::GetContainerCount() const
{
    return (sal_uInt16)pImpl->aList.Count();
}

//----------------------------------------------------------------

SfxFilterContainer* SfxFilterMatcher::GetContainer( sal_uInt16 nPos ) const
{
    return pImpl->aList.GetObject( nPos );
}

//----------------------------------------------------------------

SfxFilterContainer* SfxFilterMatcher::GetContainer( const String &rName ) const
{
    SfxFContainerList_Impl& rList = pImpl->aList;
    sal_uInt16 nCount = (sal_uInt16) rList.Count();
    for( sal_uInt16 n = 0; n < nCount; n++ )
        if( rList.GetObject( n )->GetName() == rName )
            return rList.GetObject( n );
    return 0;
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilterIgnoringContent(
    SfxMedium& rMedium, const SfxFilter**ppFilter,
    SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    String aFileName = rMedium.GetName();
    sal_uInt32 nErr = ERRCODE_NONE;
    const SfxFilter* pFilter = *ppFilter;
    const INetURLObject& rObj = rMedium.GetURLObject();

    if( !pFilter )
        pFilter = SFX_APP()->GetFilterMatcher().GetFilter4Protocol( rMedium );

    sal_Bool bCheckExternBrowser = sal_False;
    if( !pFilter )
    {
        // Falls Medium Remote, Zunaechst ueber Mimetypen pruefen ( nicht bei ExternBrowser, sofern dabei angeladen wird )
        if( !pFilter && rMedium.SupportsMIME_Impl() && ( !bCheckExternBrowser || rObj.GetProtocol() != INET_PROT_HTTP && rObj.GetProtocol() != INET_PROT_HTTPS ) )
        {
            // Mime Typen holen
            String aMime;
            nErr = rMedium.GetMIMEAndRedirect( aMime );
            nErr = ERRCODE_TOERROR( nErr );
            if ( nErr == ERRCODE_IO_PENDING )
                return nErr;
            else if( !nErr && aMime.Len() )
            {
                if ( aMime.EqualsAscii(CONTENT_TYPE_STR_X_CNT_HTTPFILE) )
                    // FilterDetection nur "uber den Content
                    return ERRCODE_NONE;
                else
                    pFilter = GetFilter4Mime( aMime, nMust, nDont );

                if ( pFilter && aMime.EqualsAscii(CONTENT_TYPE_STR_APP_OCTSTREAM) )
                {
                    // Damit eigene Formate wie sdw auch bei falsch konfiguriertem Server erkannt werden, bevor
                    // wir GetInStream rufen
                    const SfxFilter* pMimeFilter = pFilter;
                    pFilter = GetFilter4Extension( rMedium.GetURLObject().GetName(), nMust, nDont );
                    if( pFilter )
                        nErr = ERRCODE_NONE;
                    else
                        pFilter = pMimeFilter;
                }

                // Bei MIME Typen keinen Storage anfordern
                if( !nErr && rMedium.SupportsMIME_Impl() && pFilter && !pFilter->UsesStorage() )
                {
                    rMedium.GetInStream();
                    nErr = rMedium.GetError();
                }
            }

            if( nErr )
            {
                if ( nErr == ERRCODE_SFX_CONSULTUSER )
                    *ppFilter = pFilter;
                return nErr;
            }
            else if ( pFilter && aMime.EqualsAscii(CONTENT_TYPE_STR_TEXT_HTML) )
            {
                // MIME ist vertrauenswuerdig ( !? )
                nErr = ERRCODE_SFX_NEVERCHECKCONTENT;
            }
        }

        if( !pFilter && rMedium.IsDownloadDone_Impl() )
        {
            // dann ueber Storage CLSID
            // Remote macht das keinen Sinn, wenn der Download noch la"uft
            SvStorageRef aStor = rMedium.GetStorage();
            if ( aStor.Is() )
                pFilter = GetFilter4ClipBoardId( aStor->GetFormat(), nMust, nDont );

            // Als naechstes ueber Extended Attributes pruefen
            String aNewFileName;
            if( !pFilter )
            {
                aNewFileName = rMedium.GetPhysicalName();
                SvEaMgr aMgr( aNewFileName );
                String aType;
                if( aMgr.GetFileType( aType ))
                {
                    // Plain text verwerfen wir, damit unter OS/2
                    // html Dateien mit attribut Text als html geladen werden
                    if( aType.CompareIgnoreCaseToAscii( "Plain Text" ) != COMPARE_EQUAL )
                        pFilter = GetFilter4EA( aType, nMust, nDont );
                }
            }
        }

        // Zu allerletzt ueber Extension mappen
        if( !pFilter )
        {
            pFilter = GetFilter4Extension( rMedium.GetURLObject().GetName(), nMust, nDont );
            if( !pFilter || pFilter->GetWildcard()==DEFINE_CONST_UNICODE("*.*") || pFilter->GetWildcard() == '*' )
                pFilter = 0;
        }
    }

    *ppFilter = pFilter;
    return nErr;
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
        aText += ByteString(U2S(pFilter->GetName()));           \
    }                                                           \
    DBG_ERROR( aText.GetBuffer() );                             \
    nErr = ERRCODE_ABORT;                                       \
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilter(
    SfxMedium& rMedium, const SfxFilter**ppFilter,
    SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    const SfxFilter* pOldFilter = *ppFilter;
    const SfxFilter* pFilter = pOldFilter;

    sal_Bool bConsultUser = sal_False;
    sal_Bool bSupportsMime = rMedium.SupportsMIME_Impl();

    // Zunaechst, falls Filter mitkommt einmal testen, ob dieser in Ordnung ist.
    ErrCode nErr = ERRCODE_NONE;
    if( pFilter && ( pFilter->GetFilterContainer()->GetFlags() & SFX_FILTER_CONTAINER_FACTORY ) )
    {
        rMedium.StartDownload();
        if ( !rMedium.IsDownloadDone_Impl() )
        {
            if ( pFilter->GetFilterFlags() & SFX_FILTER_ASYNC )
            {
                // kurzzeitig auf synchron schalten solange die Apps sich weigern asynchron
                // zu detecten
                rMedium.ForceSynchronStream_Impl( sal_True );
                if ( !rMedium.GetInStream() )
                {
                    ErrCode e = rMedium.GetErrorCode();
                    if ( e == ERRCODE_NONE )
                    {
                        *ppFilter = pFilter;
                        return ERRCODE_IO_PENDING;
                    }
                    else
                        return e;
                }
            }
            else
            {
                 *ppFilter = pFilter;
                return ERRCODE_IO_PENDING;
            }
        }

        if( bSupportsMime && !pFilter->UsesStorage() )
            rMedium.GetInStream();

        nErr = pFilter->GetFilterContainer()->GetFilter4Content( rMedium, &pFilter, nMust, nDont );
        CHECKERROR();

        rMedium.ForceSynchronStream_Impl( sal_False );

        // ABORT bedeutet Filter ungueltig
        if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
            return nErr;

        // War der Filter ungueltig oder wurde ein anderer zurueckgegeben,
        // so detecten wir selbst (wg. redirection)
        if( nErr == ERRCODE_ABORT && pFilter )
            bConsultUser = sal_True;

        if( nErr != ERRCODE_SFX_FORCEQUIET && pOldFilter && pFilter != pOldFilter )
            pFilter = 0;
    }

    if( !pFilter )
    {
        bConsultUser = sal_False;
//DV !!!! don't close InStream when using the new medium
//      rMedium.CloseInStream();

        // Als erstes Protocol, MIME-Type, Extension etc. probieren
        nErr = GuessFilterIgnoringContent( rMedium, &pFilter, nMust, nDont );
        if ( nErr == ERRCODE_IO_PENDING )
        {
            *ppFilter = pFilter;
            return nErr;
        }

        if ( pFilter && nErr == ERRCODE_SFX_CONSULTUSER )
            *ppFilter = pFilter;

        if( nErr && nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET && nErr != ERRCODE_SFX_NEVERCHECKCONTENT )
            return nErr;

        if( nErr == ERRCODE_ABORT )
            pFilter = 0;

        // Jetzt wird geprueft, ob das Modul auch einverstanden ist; ist das nicht der Fall, wird auf
        // jeden Fall auf ConsultUser umgeschaltet
        if( pFilter )
        {
            if( nErr == ERRCODE_SFX_NEVERCHECKCONTENT )
                nErr = ERRCODE_NONE;
            else if( pFilter->GetFilterContainer()->GetFlags() & SFX_FILTER_CONTAINER_FACTORY )
            {
                rMedium.StartDownload();
                if ( !rMedium.IsDownloadDone_Impl() )
                {
                    if ( !pFilter->UsesStorage() && rMedium.GetInStream() && ( pFilter->GetFilterFlags() & SFX_FILTER_ASYNC ) )
                        // kurzzeitig auf synchron schalten solange die Apps sich weigern asynchron
                        // zu detecten
                        rMedium.ForceSynchronStream_Impl( sal_True );
                    else
                    {
                        *ppFilter = pFilter;
                        return ERRCODE_IO_PENDING;
                    }
                }

                const SfxFilter* pTmpFilter = pFilter;
                nErr = pFilter->GetFilterContainer()->GetFilter4Content( rMedium, &pFilter, nMust, nDont );
                CHECKERROR();

                rMedium.ForceSynchronStream_Impl( sal_False );

                // ABORT bedeutet Filter ungueltig
                if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
                     return nErr;

                if( nErr == ERRCODE_ABORT && pFilter )
                    pFilter = 0;
            }
        }
    }

    // Jetzt einmal drueberiterieren und nur die perfekten Filter nehmen
    if( !pFilter )
    {
        // Achtung: hier k"onnte auch asynchron detected werden!
        rMedium.StartDownload();
        if ( !rMedium.IsDownloadDone_Impl() )
            return ERRCODE_IO_PENDING;

        rMedium.GetStorage();
        nErr = rMedium.GetErrorCode();
        if( nErr )
            return nErr;
        nErr = GetFilter4Content( rMedium, &pFilter, nMust, nDont, sal_True );
        CHECKERROR();
        if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
            return nErr;

        if( nErr == ERRCODE_ABORT && pFilter )
            pFilter = 0;
    }

    // Letzte Moeglichkeit ist die Befragung aller ObjectFactories.
    if( !pFilter )
    {
        // Achtung: hier k"onnte auch asynchron detected werden!
        if ( !rMedium.IsDownloadDone_Impl() )
            return ERRCODE_IO_PENDING;

//DV !!!! don't close InStream when using the new Medium
//rMedium.CloseInStream();

        rMedium.GetStorage();
        nErr = GetFilter4Content( rMedium, &pFilter, nMust, nDont );
        CHECKERROR();
        if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
            return nErr;

        if( nErr == ERRCODE_ABORT && pFilter )
            bConsultUser = sal_True;
    }

    *ppFilter = pFilter;

    if ( ( ( pOldFilter && pOldFilter!=pFilter &&
            ( !pOldFilter->IsOwnTemplateFormat() || !pFilter->IsOwnFormat() ) ) ) &&
             nErr != ERRCODE_SFX_FORCEQUIET || bConsultUser )
        return ERRCODE_SFX_CONSULTUSER;

    if( !pOldFilter )
        pOldFilter = pFilter;

    // Checken, ob Daten vorliegen. Dies verhindert Fehler, die in
    // GetFilter4Content auftreten und die Auswahlbox hochbringen.
    ErrCode nMediumError = rMedium.GetErrorCode();
    if( nMediumError )
        return nMediumError;

    *ppFilter = pFilter;
    if ( ( nErr ||
          ( pOldFilter && pOldFilter!=pFilter
            && ( !pOldFilter->IsOwnTemplateFormat() ||
                 !pFilter->IsOwnFormat() ) ))
             && nErr != ERRCODE_SFX_FORCEQUIET )
        nErr = ERRCODE_SFX_CONSULTUSER;

    if( nErr == ERRCODE_SFX_FORCEQUIET )
        nErr = 0;
    return nErr;
};

//----------------------------------------------------------------
sal_Bool SfxFilterMatcher::IsFilterInstalled( const SfxFilter* pFilter )
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
        if( !IsFilterInstalled( pOldFilter ) )
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
            if ( IsFilterInstalled( pInstallFilter ) )
                // Eventuell wurde der Filter nachinstalliert
                pFilter = pInstallFilter;
        }
        else
        {
            // Jetzt auch Filter testen, die erst von Star bezogen werden m"ussen ( ErrCode ist irrelevant )
            GuessFilter( rMedium, &pInstallFilter, SFX_FILTER_IMPORT, 0 );
            if ( pInstallFilter )
                IsFilterInstalled( pInstallFilter );
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

    *ppFilter = pFilter;

    if ( bHidden || bAPI && nErr == ERRCODE_SFX_CONSULTUSER )
        nErr = pFilter ? ERRCODE_NONE : ERRCODE_ABORT;
    return nErr;
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcher::GetDefaultFilter() const
/*  [Beschreibung]

    Returned den ersten Filter, der nicht internal ist und im Filedialog
    auftaucht
    */
{
    SfxFilterMatcherIter aIter(
        this, 0, SFX_FILTER_INTERNAL | SFX_FILTER_NOTINFILEDLG );
    return aIter.First();
}

//----------------------------------------------------------------

sal_uInt32 SfxFilterMatcher::GetFilter4Content(
    SfxMedium& rMedium, const SfxFilter** ppFilter,  SfxFilterFlags nMust, SfxFilterFlags nDont, sal_Bool bOnlyGoodOnes ) const
{
    sal_uInt32 nErr = ERRCODE_NONE;
    SfxFContainerList_Impl& rList = pImpl->aList;
    sal_uInt16 nCount = (sal_uInt16)rList.Count();
    for( sal_uInt16 n = 0; n<nCount; n++ )
    {
        const SfxFilter* pFilter = 0;
        nErr = rList.GetObject( n )->GetFilter4Content(
            rMedium, &pFilter, nMust, nDont );
        CHECKERROR();
        if( nErr == ERRCODE_ABORT && bOnlyGoodOnes ) pFilter = 0;
        if( pFilter )
        {
            *ppFilter = pFilter;
            return nErr;
        }
    }
    return 0;
}

//----------------------------------------------------------------
/*
const SfxFilter* SfxFilterMatcher::GetFilter4ClipBoardId( sal_uInt32 nFormat, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    const SfxFilter* pFirstFilter = 0;
    SfxApplication* pApp = SFX_APP();
    if( this == &pApp->GetFilterMatcher() )
        pApp->ForcePendingInitFactories();
    SfxFContainerList_Impl& rList = pImpl->aList;
    sal_uInt16 nCount = (sal_uInt16)rList.Count();
    for( sal_uInt16 n = 0; n<nCount; n++ )
    {
        const SfxFilter* pFilter =
            rList.GetObject( n )->GetFilter4ClipBoardId(nFormat, nMust, nDont );
        if( pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_PREFERED ) )
            return pFilter;
        else if ( !pFirstFilter )
            pFirstFilter = pFilter;
    }
    return pFirstFilter;
}
*/
#define IMPL_LOOP( Type, ArgType )                              \
const SfxFilter* SfxFilterMatcher::Type(                        \
    ArgType rStr, SfxFilterFlags nMust, SfxFilterFlags nDont ) const \
{                                                               \
    const SfxFilter* pFirstFilter = 0;                          \
    SfxApplication* pApp = SFX_APP();                           \
    if( this == &pApp->GetFilterMatcher() )                     \
        pApp->ForcePendingInitFactories();                      \
    SfxFContainerList_Impl& rList = pImpl->aList;               \
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
}

IMPL_LOOP( GetFilter4Mime, const String& )
IMPL_LOOP( GetFilter4EA, const String& )
IMPL_LOOP( GetFilter4Extension, const String& )
IMPL_LOOP( GetFilter4Protocol, SfxMedium& )
IMPL_LOOP( GetFilter4ClipBoardId, sal_uInt32 )
IMPL_LOOP( GetFilter, const String& )
IMPL_LOOP( GetFilter4FilterName, const String& )
IMPL_LOOP( GetFilter4UIName, const String& )

//----------------------------------------------------------------

sal_uInt32 SfxFilterMatcher::AppDetectFilter(
    SfxMedium& rMedium, const SfxFilter** ppFilter,  SfxFilterFlags nMust, SfxFilterFlags nDont )
{
    SfxFilterFlags nClass = nMust & SFX_FILTER_TEMPLATE ?
        SFX_FILTER_TEMPLATE : USHRT_MAX;
    return SFX_APP()->DetectFilter( rMedium, ppFilter, (sal_uInt16)nMust );
}

//----------------------------------------------------------------

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
    : pMatch( pMatchP->pImpl),
      nOrMask( nOrMaskP ), nAndMask( nAndMaskP )
{
    // Iterator auf AppFilterMatcher -> DoInitFactory
    SfxApplication* pApp = SFX_APP();
    if( pMatchP == &pApp->GetFilterMatcher() )
        pApp->ForcePendingInitFactories();
    if( nOrMask == 0xffff ) //Wg. Fehlbuild auf s
        nOrMask = 0;
}

//----------------------------------------------------------------

const SfxFilter *SfxFilterMatcherIter::Forward_Impl()
{
    sal_uInt16 nCount = (sal_uInt16 )pMatch->aList.Count();
    for( ++nAktContainer; nAktContainer < nCount ; nAktContainer++ )
    {
        pCont = pMatch->aList.GetObject( nAktContainer );
        sal_uInt16 nCnt = pCont->GetFilterCount();
        if( nCnt )
        {
            nBorder=nCnt;
            nAktFilter = 0;
            return pCont->GetFilter( 0 );
        }
    }
    return 0;
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::First_Impl()
{
    nAktFilter = -1;
    nAktContainer = -1;
    nBorder = 0;

    return Forward_Impl();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::Next_Impl()
{
    if( ++nAktFilter < nBorder )
        return pCont->GetFilter( nAktFilter );
    return Forward_Impl();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::First()
{
    const SfxFilter* pFilter;
    for( pFilter = First_Impl(); pFilter; pFilter = Next_Impl())
    {
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if( ((nFlags & nOrMask) == nOrMask ) && !(nFlags & nAndMask ) )
            break;
    }
    return pFilter;
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::Next()
{
    const SfxFilter* pFilter;
    for( pFilter = Next_Impl(); pFilter; pFilter = Next_Impl())
    {
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if( ((nFlags & nOrMask) == nOrMask ) && !(nFlags & nAndMask ) )
            break;
    }
    return pFilter;
}

sal_Bool SfxFilterContainer::IsUsableForRedirects() const
{
    return FALSE;
}

const SfxFilter* SfxExecutableFilterContainer::GetDownloadFilter()
{
    return NULL;
}

const SfxFilter* SfxExecutableFilterContainer::GetChooserFilter()
{
    return NULL;
}

void SfxFilterContainer::ReadExternalFilters( const String& rDocServiceName )
{
 //   RealLoad_Impl();
    try
    {
        // get the FilterFactory service to access the registered filters
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
        ::com::sun::star::uno::Reference < ::com::sun::star::container::XNameAccess > xFilters (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );

        // get the TypeDetection service to access all registered types
        ::com::sun::star::uno::Reference < ::com::sun::star::container::XNameAccess > xTypes (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" ) ), ::com::sun::star::uno::UNO_QUERY );

        // get names of all registered filters
        ::com::sun::star::uno::Sequence < ::rtl::OUString > aNames = xFilters->getElementNames();
        sal_Int32 nLength = aNames.getLength();
        for ( sal_Int32 nName=0; nName<nLength; nName++ )
        {
            // iterate filters; each filter is described by a property value sequence
            ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aProps;
            ::rtl::OUString aName( aNames[nName] );
            if ( xFilters->hasByName( aName ) )
                xFilters->getByName( aName ) >>= aProps;

            sal_Int32 nFilterFlags = 0, nClipId = 0, nDocIconId = 0, nVersion = 0;
            ::rtl::OUString aMimeType, aType, aUIName, aDefaultTemplate, aHumanName;
            String aFilterName, aUserData;
            String aExtension, aWildCard( DEFINE_CONST_UNICODE("*.") );
            BOOL bMatches = FALSE;

            // evaluate properties : get document service name
            sal_Int32 nFilterProps = aProps.getLength();
            for ( sal_Int32 nFilterProp = 0; nFilterProp<nFilterProps; nFilterProp++ )
            {
                const ::com::sun::star::beans::PropertyValue& rFilterProp = aProps[nFilterProp];
                if ( rFilterProp.Name.compareToAscii("DocumentService") == COMPARE_EQUAL )
                {
                    ::rtl::OUString aTmp;
                    rFilterProp.Value >>= aTmp;
                    if ( aTmp.equals( rDocServiceName ) )
                        // it's me!
                        bMatches = TRUE;
                    else
                        break;
                }
                else if ( rFilterProp.Name.compareToAscii("Type") == COMPARE_EQUAL )
                {
                    if ( ( rFilterProp.Value >>= aType ) && aType.getLength() )
                    {
                        // get the Type; each type is described by a property value sequence too
                        ::com::sun::star::uno::Any aTmp;
                        if ( !xTypes->hasByName( aType ) || !( xTypes->getByName( aType ) >>= aTmp ) )
                            break;

                        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aTypeProps;
                        if ( aTmp >>= aTypeProps )
                        {
                            sal_Int32 nLength = aTypeProps.getLength();
                            for ( sal_Int32 nProp = 0; nProp<nLength; nProp++ )
                            {
                                const ::com::sun::star::beans::PropertyValue& rProp = aTypeProps[nProp];

                                if ( rProp.Name.compareToAscii("MediaType") == COMPARE_EQUAL )
                                    rProp.Value >>= aMimeType;
                                else if ( rProp.Name.compareToAscii("Extensions") == COMPARE_EQUAL )
                                {
                                    ::com::sun::star::uno::Sequence < ::rtl::OUString > aExtensions;
                                    if ( rProp.Value >>= aExtensions )
                                    {
                                        sal_Int32 nExtensions = aExtensions.getLength();
                                        const ::rtl::OUString* pExtensions = aExtensions.getConstArray();
                                        for ( sal_Int32 nExt=0; nExt<nExtensions; nExt++ )
                                        {
                                            aExtension += aWildCard;
                                            aExtension += (String) pExtensions[nExt];
                                            if ( nExt+1 < nExtensions )
                                                aExtension += ';';
                                        }
                                    }
                                }
                                else if ( rProp.Name.compareToAscii("ClipboardFormat") == COMPARE_EQUAL )
                                    rProp.Value >>= aHumanName;
                                else if ( rProp.Name.compareToAscii("DocumentIconId") == COMPARE_EQUAL )
                                    rProp.Value >>= nClipId;
                            }
                        }
                    }
                }
                else if ( rFilterProp.Name.compareToAscii("UIName") == COMPARE_EQUAL )
                    rFilterProp.Value >>= aUIName;
                else if ( rFilterProp.Name.compareToAscii("TemplateName") == COMPARE_EQUAL )
                    rFilterProp.Value >>= aDefaultTemplate;
                else if ( rFilterProp.Name.compareToAscii("FileFormatVersion") == COMPARE_EQUAL )
                    rFilterProp.Value >>= nVersion;
                else if ( rFilterProp.Name.compareToAscii("UserData") == COMPARE_EQUAL )
                {
                    Sequence < ::rtl::OUString > aUserDataList;
                    if ( rFilterProp.Value >>= aUserDataList )
                    {
                        sal_Int32 nStrings = aUserDataList.getLength();
                        for ( sal_Int32 nString=0; nString<nStrings; nString++ )
                        {
                            aUserData += String( aUserDataList[nString] );
                            if ( nString+1 < nStrings )
                                aUserData += ',';
                        }
                    }
                }
                else if ( rFilterProp.Name.compareToAscii("Flags") == COMPARE_EQUAL )
                    rFilterProp.Value >>= nFilterFlags;
            }

            if ( bMatches && aType.getLength() )
            {
/*
                ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
                aDataFlavor.MimeType = aMimeType;
                nClipId = SotExchange::GetFormat( aDataFlavor );
 */
                if ( !nClipId && aHumanName.getLength() )
                {
                    // old formats are found using HumanPresentableName
                    nClipId = SotExchange::RegisterFormatName( aHumanName );
                }

                // register SfxFilter
                aFilterName = impl_getOldFilterName( aName );
                if ( aFilterName.Len() )
                {
#ifdef DBG_UTIL
                    if ( !aUserData.Len() )
                    {
                        aUserData = impl_getUserData( aFilterName );
                        DBG_ASSERT( !aUserData.Len(), "Wrong UserData in Configuration!" )
                    }
#endif
                    USHORT nPos = aFilterName.Search( ':' );
                    aFilterName.Erase( 0, nPos+2 );
                }
                else
                    aFilterName = aName;

                SfxFilter *pFilter = new SfxFilter( aFilterName, aExtension, nFilterFlags, nClipId, aType,
                        aType, (USHORT) nDocIconId, aMimeType, this, aUserData );

                pFilter->SetUIName( aUIName );
                pFilter->SetDefaultTemplate( aDefaultTemplate );
                if( nVersion )
                    pFilter->SetVersion( nVersion );
                BOOL bIsDefault = (( nFilterFlags & SFX_FILTER_DEFAULT ) != 0 );
                AddFilter( pFilter, bIsDefault ? 0 : GetFilterCount() );
            }
        }
    }

    catch ( ::com::sun::star::uno::Exception& )
    {
    }
}

String SfxFilterContainer::ConvertToOldFilterName( const String& aNew )
{
    return impl_getOldFilterName( aNew );
}

String SfxFilterContainer::ConvertToNewFilterName( const String& aOld )
{
    return impl_getNewFilterName( aOld );
}
