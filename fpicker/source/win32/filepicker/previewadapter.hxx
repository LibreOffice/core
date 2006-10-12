/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: previewadapter.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:54:28 $
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

#ifndef _PREVIEWADAPTER_HXX_
#define _PREVIEWADAPTER_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <memory>

// forward declaration
class CPreviewAdapterImpl;

//---------------------------------------------
// A kind of a facade for the preview class.
// We want to hide the fact that the preview
// window may only become visible if there is
// a valid parent window (means, the FilePicker)
// is in execution mode. So unless someone sets
// the preview active with a valid parent
// window the preview may not be visible
//---------------------------------------------

class CPreviewAdapter
{
public:

    // ctor
    CPreviewAdapter(HINSTANCE instance);

    ~CPreviewAdapter();

    ::com::sun::star::uno::Sequence<sal_Int16> SAL_CALL getSupportedImageFormats();

    sal_Int32 SAL_CALL getTargetColorDepth();

    sal_Int32 SAL_CALL getAvailableWidth();

    sal_Int32 SAL_CALL getAvailableHeight();

    void SAL_CALL setImage(sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL setShowState(sal_Bool bShowState);

    sal_Bool SAL_CALL getShowState();

    void SAL_CALL setParent(HWND parent);

    //--------------------------------------
    // notification from parent
    //--------------------------------------

    void SAL_CALL notifyParentShow(bool bShow);

    void SAL_CALL notifyParentSizeChanged();

    void SAL_CALL notifyParentWindowPosChanged();

private:
    // hide implementation details using the
    // bridge pattern
    std::auto_ptr<CPreviewAdapterImpl> m_pImpl;

// prevent copy and assignment
private:
    CPreviewAdapter(const CPreviewAdapter&);
    CPreviewAdapter& operator=(const CPreviewAdapter&);
};


#endif
