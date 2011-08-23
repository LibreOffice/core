/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifdef _MSC_VER
#pragma hdrstop
#endif

// HINTIDs must be on top; it is required for the macitem.hxx header
#ifndef _HINTIDS_HXX
#include "hintids.hxx"
#endif

#ifndef _UNOEVENT_HXX
#include "unoevent.hxx"
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _UNOFRAME_HXX
#include "unoframe.hxx"
#endif

#ifndef _UNOSTYLE_HXX
#include "unostyle.hxx"
#endif

#ifndef _SWEVENT_HXX
#include "swevent.hxx"
#endif

#ifndef _DOCSTYLE_HXX
#include "docstyle.hxx"
#endif

#ifndef _SFX_HRC
#include <bf_sfx2/sfx.hrc>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif

#ifndef _FMTINFMT_HXX
#include "fmtinfmt.hxx"
#endif

#ifndef _SFXMACITEM_HXX
#include <bf_svtools/macitem.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::beans::PropertyValue;
using ::cppu::WeakImplHelper2;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


const sal_Char sAPI_ServiceName[] = "com.sun.star.container.XNameReplace";
const sal_Char sAPI_SwFrameEventDescriptor[] = "SwFrameEventDescriptor";
const sal_Char sAPI_SwFrameStyleEventDescriptor[] = 
                                    "SwFrameStyleEventDescriptor";
const sal_Char sAPI_SwDetachedEventDescriptor[] = "SwDetachedEventDescriptor";
const sal_Char sAPI_SwHyperlinkEventDescriptor[] = 
                                    "SwHyperlinkEventDescriptor";
const sal_Char sAPI_SwAutoTextEventDescriptor[] =
                                    "SwAutoTextEventDescriptor";


// //
// // tables of all known events handled by this class
// //

// // careful: keep aKnownEventIDs and aKnownEventNames in sync!

// const USHORT aKnownEventIDs[] =
// {
// 	SW_EVENT_OBJECT_SELECT,
// 	SW_EVENT_START_INS_GLOSSARY,
// 	SW_EVENT_END_INS_GLOSSARY,
// 	SW_EVENT_MAIL_MERGE,
// 	SW_EVENT_FRM_KEYINPUT_ALPHA,
// 	SW_EVENT_FRM_KEYINPUT_NOALPHA,
// 	SW_EVENT_FRM_RESIZE,
// 	SW_EVENT_FRM_MOVE,
// 	SW_EVENT_PAGE_COUNT,
// 	SFX_EVENT_MOUSEOVER_OBJECT,
// 	SFX_EVENT_MOUSECLICK_OBJECT,
// 	SFX_EVENT_MOUSEOUT_OBJECT,
//  	SFX_EVENT_OPENDOC,
//  	SFX_EVENT_CLOSEDOC,
//  	SFX_EVENT_STARTAPP,
//  	SFX_EVENT_CLOSEAPP,
//  	SFX_EVENT_CREATEDOC,
//  	SFX_EVENT_SAVEDOC,
//  	SFX_EVENT_SAVEASDOC,
//  	SFX_EVENT_ACTIVATEDOC,
//  	SFX_EVENT_DEACTIVATEDOC,
//  	SFX_EVENT_PRINTDOC,
//  	SFX_EVENT_ONERROR,
//  	SFX_EVENT_LOADFINISHED,
//  	SFX_EVENT_SAVEFINISHED,
//  	SFX_EVENT_MODIFYCHANGED,
//  	SFX_EVENT_PREPARECLOSEDOC,
//  	SFX_EVENT_NEWMESSAGE,
//  	SFX_EVENT_TOGGLEFULLSCREENMODE,
//  	SFX_EVENT_SAVEDOCDONE,
//  	SFX_EVENT_SAVEASDOCDONE,
// 	SVX_EVENT_IMAGE_LOAD,
// 	SVX_EVENT_IMAGE_ABORT,
// 	SVX_EVENT_IMAGE_ERROR,
// 	0
// };

// const sal_Char* aKnownEventNames[] =
// {
// 	"OnSelect",					// SW_EVENT_OBJECT_SELECT
// 	"OnInsertStart",			// SW_EVENT_START_INS_GLOSSARY
// 	"OnInsertDone",				// SW_EVENT_END_INS_GLOSSARY
// 	"OnMailMerge",				// SW_EVENT_MAIL_MERGE
// 	"OnAlphaCharInput",			// SW_EVENT_FRM_KEYINPUT_ALPHA
// 	"OnNonAlphaCharInput",		// SW_EVENT_FRM_KEYINPUT_NOALPHA
// 	"OnResize",					// SW_EVENT_FRM_RESIZE
// 	"OnMove",					// SW_EVENT_FRM_MOVE
// 	"PageCountChange",			// SW_EVENT_PAGE_COUNT
// 	"OnMouseOver",				// SFX_EVENT_MOUSEOVER_OBJECT
// 	"OnClick",					// SFX_EVENT_MOUSECLICK_OBJECT
// 	"OnMouseOut",				// SFX_EVENT_MOUSEOUT_OBJECT
// 	"OnLoad",				    // SFX_EVENT_OPENDOC,               
// 	"OnUnload",					// SFX_EVENT_CLOSEDOC,              
// 	"OnStartApp",				// SFX_EVENT_STARTAPP,              
// 	"OnCloseApp",				// SFX_EVENT_CLOSEAPP,              
// 	"OnNew",					// SFX_EVENT_CREATEDOC,             
// 	"OnSave",					// SFX_EVENT_SAVEDOC,               
// 	"OnSaveAs",					// SFX_EVENT_SAVEASDOC,             
// 	"OnFocus",					// SFX_EVENT_ACTIVATEDOC,           
// 	"OnUnfocus",				// SFX_EVENT_DEACTIVATEDOC,         
// 	"OnPrint",					// SFX_EVENT_PRINTDOC,              
// 	"OnError",					// SFX_EVENT_ONERROR,               
// 	"OnLoadFinished",			// SFX_EVENT_LOADFINISHED,          
// 	"OnSaveFinished",			// SFX_EVENT_SAVEFINISHED,          
// 	"OnModifyChanged",			// SFX_EVENT_MODIFYCHANGED,         
// 	"OnPrepareUnload",			// SFX_EVENT_PREPARECLOSEDOC,       
// 	"OnNewMail",				// SFX_EVENT_NEWMESSAGE,            
// 	"OnToggleFullscreen",		// SFX_EVENT_TOGGLEFULLSCREENMODE,  
// 	"OnSaveDone",				// SFX_EVENT_SAVEDOCDONE,           
// 	"OnSaveAsDone",				// SFX_EVENT_SAVEASDOCDONE,         
// 	"OnLoadError",				// SVX_EVENT_IMAGE_LOAD,            
// 	"OnLoadCancel",				// SVX_EVENT_IMAGE_ABORT,           
// 	"OnLoadDone",				// SVX_EVENT_IMAGE_ERROR,           

// 	NULL
// };


//
// tables of allowed events for specific objects
//

const struct SvEventDescription aGraphicEvents[] =
{ 
    { SW_EVENT_OBJECT_SELECT,		"OnSelect" },
    { SFX_EVENT_MOUSEOVER_OBJECT,	"OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,	"OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,	"OnMouseOut" },
    { SVX_EVENT_IMAGE_LOAD,			"OnLoadDone" },
    { SVX_EVENT_IMAGE_ABORT,		"OnLoadCancel" },
    { SVX_EVENT_IMAGE_ERROR,		"OnLoadError" },
    { 0, NULL }
};

const struct SvEventDescription aFrameEvents[] =
{ 
    { SW_EVENT_OBJECT_SELECT,		"OnSelect" },
    { SW_EVENT_FRM_KEYINPUT_ALPHA,	"OnAlphaCharInput" },
    { SW_EVENT_FRM_KEYINPUT_NOALPHA,	"OnNonAlphaCharInput" },
    { SW_EVENT_FRM_RESIZE,			"OnResize" },
    { SW_EVENT_FRM_MOVE,			"OnMove" },
    { SFX_EVENT_MOUSEOVER_OBJECT,	"OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,	"OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,	"OnMouseOut" },
    { 0, NULL }
};

const struct SvEventDescription aOLEEvents[] =
{ 
    { SW_EVENT_OBJECT_SELECT,		"OnSelect" },
    { SFX_EVENT_MOUSEOVER_OBJECT,	"OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,	"OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,	"OnMouseOut" },
    { 0, NULL }
};

const struct SvEventDescription aHyperlinkEvents[] =
{ 
    { SFX_EVENT_MOUSEOVER_OBJECT,	"OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,	"OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,	"OnMouseOut" },
    { 0, NULL }
};

const struct SvEventDescription aAutotextEvents[] =
{ 
    { SW_EVENT_START_INS_GLOSSARY,	"OnInsertStart" },
    { SW_EVENT_END_INS_GLOSSARY,	"OnInsertDone" },
    { 0, NULL }
};

const struct SvEventDescription aFrameStyleEvents[] =
{ 
    { SW_EVENT_OBJECT_SELECT,		"OnSelect" },
    { SW_EVENT_FRM_KEYINPUT_ALPHA,	"OnAlphaCharInput" },
    { SW_EVENT_FRM_KEYINPUT_NOALPHA,	"OnNonAlphaCharInput" },
    { SW_EVENT_FRM_RESIZE,			"OnResize" },
    { SW_EVENT_FRM_MOVE,			"OnMove" },
    { SFX_EVENT_MOUSEOVER_OBJECT,	"OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,	"OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,	"OnMouseOut" },
    { SVX_EVENT_IMAGE_LOAD,			"OnLoadDone" },
    { SVX_EVENT_IMAGE_ABORT,		"OnLoadCancel" },
    { SVX_EVENT_IMAGE_ERROR,		"OnLoadError" },
    { 0, NULL }
};

const struct SvEventDescription aDocumentEvents[] =
{ 
     { SFX_EVENT_STARTAPP,		"OnStartApp" },
     { SFX_EVENT_CLOSEAPP,		"OnCloseApp" },
     { SFX_EVENT_CREATEDOC,		"OnNew" },
     { SFX_EVENT_OPENDOC,		"OnLoad" },
     { SFX_EVENT_SAVEDOC,		"OnSave" },
     { SFX_EVENT_SAVEASDOC,		"OnSaveAs" },
// ??? cf. SAVEDOCDONE, SAVEASDOCDONE
     { SFX_EVENT_SAVEFINISHED,	"OnSaveFinished" },
     { SFX_EVENT_CLOSEDOC,		"OnUnload" },
     { SFX_EVENT_ACTIVATEDOC,	"OnFocus" },
     { SFX_EVENT_DEACTIVATEDOC,	"OnUnfocus" },
     { SFX_EVENT_ONERROR,		"OnError" },
    { SFX_EVENT_NEWMESSAGE,		"OnNewMail" },
     { SFX_EVENT_PRINTDOC,		"OnPrint" },
     { SW_EVENT_MAIL_MERGE,		"OnMailMerge" },
     { SW_EVENT_PAGE_COUNT,		"PageCountChange" },
// 	SFX_EVENT_LOADFINISHED,
// 	SFX_EVENT_MODIFYCHANGED,
// 	SFX_EVENT_PREPARECLOSEDOC,
// 	SFX_EVENT_TOGGLEFULLSCREENMODE,
// 	SFX_EVENT_SAVEDOCDONE,
// 	SFX_EVENT_SAVEASDOCDONE,

    { 0, NULL }
};





//
// SwHyperlinkEventDescriptor
//

SwHyperlinkEventDescriptor::SwHyperlinkEventDescriptor() :
    SvDetachedEventDescriptor(aHyperlinkEvents),
    sImplName(RTL_CONSTASCII_USTRINGPARAM(sAPI_SwHyperlinkEventDescriptor))
{
}

SwHyperlinkEventDescriptor::~SwHyperlinkEventDescriptor()
{
}

OUString SwHyperlinkEventDescriptor::getImplementationName(void) 
    throw( RuntimeException )
{
    return sImplName;
}

void SwHyperlinkEventDescriptor::copyMacrosFromINetFmt(
    const SwFmtINetFmt& aFmt)
{
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; i++)
    {
        USHORT nEvent = mpSupportedMacroItems[i].mnEvent;
        const SvxMacro* aMacro = aFmt.GetMacro(nEvent);
        if (NULL != aMacro)
            replaceByName(nEvent, *aMacro);
    }
}

void SwHyperlinkEventDescriptor::copyMacrosIntoINetFmt(
    SwFmtINetFmt& aFmt)
{
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; i++)
    {
        USHORT nEvent = mpSupportedMacroItems[i].mnEvent;
        if (hasByName(nEvent))
        {
            SvxMacro aMacro(sEmpty, sEmpty);
            getByName(aMacro, nEvent);
            aFmt.SetMacro(nEvent, aMacro);
        }
    }
}


void SwHyperlinkEventDescriptor::copyMacrosFromNameReplace(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameReplace> & xReplace)
{
    // iterate over all names (all names that *we* support)
    Sequence<OUString> aNames = getElementNames();
    sal_Int32 nCount = aNames.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        // copy element for that name
        const OUString& rName = aNames[i];
        if (xReplace->hasByName(rName))
        {
            SvBaseEventDescriptor::replaceByName(rName, 
                                                 xReplace->getByName(rName));
        }
    }
}


//
// SwFrameEventDescriptor
//

// use double cast in superclass constructor to avoid ambigous cast
SwFrameEventDescriptor::SwFrameEventDescriptor( 
    SwXTextFrame& rFrameRef ) :
        SvEventDescriptor((text::XTextFrame&)rFrameRef, aFrameEvents),
        sSwFrameEventDescriptor(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_SwFrameEventDescriptor)),
        rFrame(rFrameRef)
{
}

SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextGraphicObject& rGraphicRef ) :
        SvEventDescriptor((text::XTextContent&)rGraphicRef, aGraphicEvents),
        rFrame((SwXFrame&)rGraphicRef)
{
}

SwFrameEventDescriptor::SwFrameEventDescriptor( 
    SwXTextEmbeddedObject& rObjectRef ) :
        SvEventDescriptor((text::XTextContent&)rObjectRef, aOLEEvents),
        rFrame((SwXFrame&)rObjectRef)
{
}

SwFrameEventDescriptor::~SwFrameEventDescriptor()
{
}

void SwFrameEventDescriptor::setMacroItem(const SvxMacroItem& rItem)
{
    rFrame.GetFrmFmt()->SetAttr(rItem);
}

const SvxMacroItem& SwFrameEventDescriptor::getMacroItem()
{
    return (const SvxMacroItem&)rFrame.GetFrmFmt()->GetAttr(RES_FRMMACRO);
}

sal_uInt16 SwFrameEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

OUString SwFrameEventDescriptor::getImplementationName()
    throw( RuntimeException )
{
    return sSwFrameEventDescriptor;
}


//
// SwFrameStyleEventDescriptor
//

SwFrameStyleEventDescriptor::SwFrameStyleEventDescriptor( 
    SwXFrameStyle& rStyleRef ) :
        SvEventDescriptor((document::XEventsSupplier&)rStyleRef, 
                          aFrameStyleEvents),
        sSwFrameStyleEventDescriptor(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_SwFrameStyleEventDescriptor)),
        rStyle(rStyleRef)
{
}

SwFrameStyleEventDescriptor::~SwFrameStyleEventDescriptor()
{
}

void SwFrameStyleEventDescriptor::setMacroItem(const SvxMacroItem& rItem)
{
    // As I was told, for some entirely unobvious reason getting an
    // item from a style has to look as follows:
    SfxStyleSheetBasePool* pBasePool = rStyle.GetBasePool();
    if (pBasePool)
    {
        SfxStyleSheetBase* pBase = pBasePool->Find(rStyle.GetStyleName());
        if (pBase)
        {
            SwDocStyleSheet aStyle(*(SwDocStyleSheet*)pBase);
            SfxItemSet& rStyleSet = aStyle.GetItemSet();
            SfxItemSet aSet(*rStyleSet.GetPool(), RES_FRMMACRO, RES_FRMMACRO);
            aSet.Put(rItem);
            aStyle.SetItemSet(aSet);
        }
    }
}

static const SvxMacroItem aEmptyMacroItem(RES_FRMMACRO);

const SvxMacroItem& SwFrameStyleEventDescriptor::getMacroItem()
{
    // As I was told, for some entirely unobvious reason getting an
    // item from a style has to look as follows:
    SfxStyleSheetBasePool* pBasePool = rStyle.GetBasePool();
    if (pBasePool)
    {
        SfxStyleSheetBase* pBase = pBasePool->Find(rStyle.GetStyleName());
        if (pBase)
        {
            SwDocStyleSheet aStyle(*(SwDocStyleSheet*)pBase);
            return (const SvxMacroItem&)aStyle.GetItemSet().Get(RES_FRMMACRO);
        }
        else
            return aEmptyMacroItem;
    }
    else
        return aEmptyMacroItem;
}

OUString SwFrameStyleEventDescriptor::getImplementationName() 
    throw( RuntimeException )
{
    return sSwFrameStyleEventDescriptor;
}

sal_uInt16 SwFrameStyleEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

}
