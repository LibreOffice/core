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
#ifndef _SVX_FMSHIMP_HXX
#define _SVX_FMSHIMP_HXX


#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_NAVIGATIONBARMODE_HPP_
#include <com/sun/star/form/NavigationBarMode.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif



#ifndef _SVDMARK_HXX
#include "svdmark.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif


#define _SVSTDARR_BOOLS
#define _SVSTDARR_BYTES
#define _SVSTDARR_LONGS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_USHORTS

#ifndef _FMSEARCH_HXX
#include "fmsearch.hxx"
#endif
#ifndef _CPPUHELPER_COMPBASE6_HXX_
#include <cppuhelper/compbase6.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif

#include <queue>
namespace binfilter {

SV_DECL_PTRARR(SdrObjArray, SdrObject*, 32, 16)//STRIP008 ;
//	SV_DECL_OBJARR(FmFormArray, ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>, 32, 16);
DECLARE_STL_VECTOR( ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > ,FmFormArray);

// catch databse exceptions if occur
#define DO_SAFE(statement) try { statement; } catch( const Exception& ) { DBG_ERROR("unhandled exception (I tried to move a cursor (or something like that).)"); }

#define GA_DISABLE_SYNC		1
#define GA_FORCE_SYNC		2
#define GA_ENABLE_SYNC		3
#define GA_SYNC_MASK		3
#define GA_DISABLE_ROCTRLR	4
#define GA_ENABLE_ROCTRLR	8


// flags for controlling the behaviour when calling loadForms
#define FORMS_LOAD			0x0000		// default: simply load
#define FORMS_SYNC			0x0000		// default: do in synchronous

#define FORMS_UNLOAD		0x0001		// unload
#define FORMS_RESET			0x0002		// reset after the load/unload
#define FORMS_ASYNC			0x0004		// do this async

// forward declarations
class FmFormShell;


//==============================================================================
// a class iterating through all fields of a form which are bound to a field
// sub forms are ignored, grid columns (where the grid is a direct child of the form) are included

//========================================================================
// I would prefer this to be a struct local to FmXFormShell but unfortunately local structs/classes
// are somewhat difficult with some of our compilers
class FmCursorActionThread;
struct CursorActionDescription
{
    FmCursorActionThread*	pThread;
    sal_uInt32					nFinishedEvent;
        // we want to do the cleanup of the thread in the main thread so we post an event to ourself
    sal_Bool					bCanceling;
        // this thread is being canceled

    CursorActionDescription() : pThread(NULL), nFinishedEvent(0), bCanceling(sal_False) { }
};

class FmFormPage;
//========================================================================
struct FmLoadAction
{
    FmFormPage*	pPage;
    sal_uInt32	nEventId;
    sal_uInt16	nFlags;

    FmLoadAction( ) : pPage( NULL ), nFlags( 0 ), nEventId( 0 ) { }
    FmLoadAction( FmFormPage* _pPage, sal_uInt16 _nFlags, sal_uInt32 _nEventId )
        :pPage( _pPage ), nFlags( _nFlags ), nEventId( _nEventId )
    {
    }
};

//========================================================================
class SfxViewFrame;
typedef ::cppu::WeakComponentImplHelper6<	::com::sun::star::sdbc::XRowSetListener,         
                                            ::com::sun::star::beans::XPropertyChangeListener,
                                            ::com::sun::star::util::XModifyListener,         
                                            ::com::sun::star::container::XContainerListener, 
                                            ::com::sun::star::view::XSelectionChangeListener,
                                            ::com::sun::star::form::XFormControllerListener> FmXFormShell_BD_BASE;

//========================================================================
class FmXFormShell_Base_Disambiguation : public FmXFormShell_BD_BASE
{
protected:
    FmXFormShell_Base_Disambiguation( ::osl::Mutex& _rMutex ); 
    virtual void SAL_CALL disposing();
};

//========================================================================
typedef FmXFormShell_Base_Disambiguation	FmXFormShell_BASE;
typedef ::utl::ConfigItem					FmXFormShell_CFGBASE;



//------------------------------------------------------------------------------

#define DECL_CURSOR_ACTION_THREAD(classname)			\
                                                        \
class classname : public FmCursorActionThread			\
{														\
public:													\
    classname(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDataSource,		\
            const UniString& _rPath);						\
protected:												\
    virtual void RunImpl();								\
};														\


//------------------------------------------------------------------------------

#define IMPL_CURSOR_ACTION_THREAD(classname, caption, action)	\
                                                                \
classname::classname(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDataSource,			\
        const UniString& _rPath)									\
    :FmCursorActionThread(_xDataSource, caption, _rPath)		\
{																\
}																\
                                                                \
void classname::RunImpl()										\
{																\
    m_xDataSource->action;										\
}																\



}//end of namespace binfilter
#endif          // _SVX_FMSHIMP_HXX
