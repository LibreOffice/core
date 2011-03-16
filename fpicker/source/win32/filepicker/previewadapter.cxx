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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

#include <tchar.h>
#include "previewadapter.hxx"

#ifndef _COM_SUN_STAR_UI_DIALOG_FILEPREVIEWIMAGEFORMATS_HPP_
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#endif
#include "dibpreview.hxx"
#include "../misc/WinImplHelper.hxx"

#include <memory>
#include <stdexcept>

//---------------------------------------------
//
//---------------------------------------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//---------------------------------------------
// An impl class to hide implementation details
// from clients
//---------------------------------------------

class CPreviewAdapterImpl
{
public:
    CPreviewAdapterImpl(HINSTANCE instance);

    virtual ~CPreviewAdapterImpl();

    virtual sal_Int32 SAL_CALL getTargetColorDepth();

    virtual sal_Int32 SAL_CALL getAvailableWidth();

    virtual sal_Int32 SAL_CALL getAvailableHeight();

    virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const Any& aImage )
        throw (IllegalArgumentException,RuntimeException);

    virtual sal_Bool SAL_CALL setShowState(sal_Bool bShowState);

    virtual sal_Bool SAL_CALL getShowState();

    virtual void SAL_CALL setParent(HWND parent);

    virtual HWND SAL_CALL getParent();

    //-------------------------------------
    // parent notification handler
    //-------------------------------------

    virtual void SAL_CALL notifyParentShow(sal_Bool bShow);

    virtual void SAL_CALL notifyParentSizeChanged();

    virtual void SAL_CALL notifyParentWindowPosChanged();

protected:
    virtual void SAL_CALL calcRightMargin();

    virtual void SAL_CALL rearrangeLayout();

    void SAL_CALL initializeActivePreview() throw(std::runtime_error);

    HWND SAL_CALL findFileListbox() const;

// member
protected:
    HINSTANCE                   m_Instance;
    std::auto_ptr<PreviewBase>  m_Preview;
    HWND                        m_FileDialog;
    int                         m_RightMargin;

//prevent copy/assignment
private:
    CPreviewAdapterImpl(const CPreviewAdapterImpl&);
    CPreviewAdapterImpl& operator=(const CPreviewAdapterImpl&);
};

//-----------------------------------------
//
//-----------------------------------------

CPreviewAdapterImpl::CPreviewAdapterImpl(HINSTANCE instance) :
    m_Instance(instance),
    m_Preview(new PreviewBase()), // create dummy preview (NULL-Object pattern)
    m_FileDialog(0),
    m_RightMargin(0)
{
}

//-----------------------------------------
//
//-----------------------------------------

CPreviewAdapterImpl::~CPreviewAdapterImpl()
{
}

//-----------------------------------------
//
//-----------------------------------------

sal_Int32 SAL_CALL CPreviewAdapterImpl::getTargetColorDepth()
{
    return m_Preview->getTargetColorDepth();
}

//-----------------------------------------
//
//-----------------------------------------

sal_Int32 SAL_CALL CPreviewAdapterImpl::getAvailableWidth()
{
    return m_Preview->getAvailableWidth();
}

//-----------------------------------------
//
//-----------------------------------------

sal_Int32 SAL_CALL CPreviewAdapterImpl::getAvailableHeight()
{
    return m_Preview->getAvailableHeight();
}

//-----------------------------------------
//
//-----------------------------------------

void SAL_CALL CPreviewAdapterImpl::setImage( sal_Int16 aImageFormat, const Any& aImage )
    throw (IllegalArgumentException,RuntimeException)
{
    m_Preview->setImage(aImageFormat,aImage);
}

//-----------------------------------------
//
//-----------------------------------------

sal_Bool SAL_CALL CPreviewAdapterImpl::setShowState( sal_Bool bShowState )
{
    sal_Bool bRet = m_Preview->setShowState(bShowState);
    rearrangeLayout();
    return bRet;
}

//-----------------------------------------
//
//-----------------------------------------

sal_Bool SAL_CALL CPreviewAdapterImpl::getShowState()
{
    return m_Preview->getShowState();
}

//-----------------------------------------
//
//-----------------------------------------

void SAL_CALL CPreviewAdapterImpl::setParent(HWND parent)
{
    OSL_PRECOND(IsWindow(parent),"Invalid FileDialog handle");

    m_FileDialog = parent;
    calcRightMargin();
}

//-----------------------------------------
//
//-----------------------------------------

HWND SAL_CALL CPreviewAdapterImpl::getParent()
{
    return m_FileDialog;
}

//-----------------------------------------
//
//-----------------------------------------

void SAL_CALL CPreviewAdapterImpl::calcRightMargin()
{
    // Calculate the right reference margin
    //
    // Assumtions:
    // 1. This method will be called before the dialog becomes
    //    visible
    // 2. There exist a FileListbox with the id lst1 even
    //    if it is not visible like under Win2000/XP
    // 3. Initially this FileListbox has the appropriate size
    //    to fit within the FileListbox
    // 4. The margin between the right edge of the FileListbox
    //    and the right edge of the FileDialog will be constant
    //    even if the size of the dialog changes

    HWND flb = GetDlgItem(m_FileDialog,lst1);

    OSL_ENSURE(IsWindow(flb),"Filelistbox not found");

    RECT rcFlb;
    GetWindowRect(flb,&rcFlb);

    RECT rcFileDlg;
    GetWindowRect(m_FileDialog,&rcFileDlg);

    m_RightMargin = rcFileDlg.right - rcFlb.right;
}

//-----------------------------------------
//
//-----------------------------------------

void SAL_CALL CPreviewAdapterImpl::notifyParentShow(sal_Bool)
{
}

//-----------------------------------------
//
//-----------------------------------------

void SAL_CALL CPreviewAdapterImpl::notifyParentSizeChanged()
{
    rearrangeLayout();
}

//-----------------------------------------
//
//-----------------------------------------

void SAL_CALL CPreviewAdapterImpl::notifyParentWindowPosChanged()
{
}

//-----------------------------------------
//
//-----------------------------------------

void SAL_CALL CPreviewAdapterImpl::rearrangeLayout()
{
    // try to get a handle to the filelistbox
    // if there is no new-style filelistbox like
    // in Win2000/XP there should be at least the
    // old listbox, so we take this one
    // lst1 - identifies the old-style filelistbox
    // lst2 - identifies the new-style filelistbox
    // see dlgs.h
    HWND flb_new = findFileListbox();

    // under Windows NT 4.0 the size of the old
    // filelistbox will be used as reference for
    // sizing the new filelistbox, so we have
    // to change the size of it too
    HWND flb_old = GetDlgItem(m_FileDialog,lst1);

    RECT rcFlbNew;
    GetWindowRect(flb_new,&rcFlbNew);

    RECT rcFileDlg;
    GetWindowRect(m_FileDialog,&rcFileDlg);
    rcFileDlg.right -= m_RightMargin;

    // the available area for the filelistbox should be
    // the left edge of the filelistbox and the right
    // edge of the OK button, we take this as reference
    int height = rcFlbNew.bottom - rcFlbNew.top;
    int width  = rcFileDlg.right - rcFlbNew.left;

    HWND prvwnd = m_Preview->getWindowHandle();

    // we use GetWindowLong to ask for the visibility
    // of the preview window because IsWindowVisible
    // only returns true the specified window including
    // its parent windows are visible
    // this is not the case when we are called in response
    // to the WM_SHOWWINDOW message, somehow the WS_VISIBLE
    // style bit of the FileOpen dialog must be set after that
    // message
    LONG lStyle = GetWindowLong(prvwnd,GWL_STYLE);
    BOOL bIsVisible = ((lStyle & WS_VISIBLE) != 0);

    int cx = 0;

    if (IsWindow(prvwnd) && bIsVisible)
    {
        cx = width/2;

        // resize the filelistbox to the half of the
        // available space
        bool bRet = SetWindowPos(flb_new,
            NULL, 0, 0, cx, height,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        bRet = SetWindowPos(flb_old,
            NULL, 0, 0, cx, height,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        // get the new dimensions of the filelistbox after
        // resizing and take the right,top corner as starting
        // point for the preview window
        GetWindowRect(flb_new,&rcFlbNew);
        POINT pt = { rcFlbNew.right, rcFlbNew.top };
        ScreenToClient(m_FileDialog,&pt);

        // resize the preview window to fit within
        // the available space and set the window
        // to the top of the z-order else it will
        // be invisible
        SetWindowPos(prvwnd,
            HWND_TOP, pt.x, pt.y, cx, height, SWP_NOACTIVATE);
    }
    else
    {
        // resize the filelistbox to the maximum available
        // space
        cx = rcFileDlg.right - rcFlbNew.left;

        // resize the old filelistbox
        SetWindowPos(flb_old,
            NULL, 0, 0, cx, height,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        // resize the new filelistbox
        SetWindowPos(flb_new,
            NULL, 0, 0, cx, height,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
    }
}

//-----------------------------------------
//
//-----------------------------------------

void SAL_CALL CPreviewAdapterImpl::initializeActivePreview() throw(std::runtime_error)
{
    sal_Bool bShowState = m_Preview->getImaginaryShowState();

    sal_Int16 aImgFrmt;
    Any aImg;
    m_Preview->getImage(aImgFrmt,aImg);

    HWND flb = findFileListbox();

    PreviewBase* prv = new CDIBPreview(
        m_Instance, GetParent(flb), bShowState);

    m_Preview.reset(prv);

    m_Preview->setImage(aImgFrmt,aImg);
}

//-----------------------------------------
//
//-----------------------------------------

HWND SAL_CALL CPreviewAdapterImpl::findFileListbox() const
{
    // try to get a handle to the filelistbox
    // if there is no new-style filelistbox like
    // in Win2000/XP there should be at least the
    // old listbox, so we take this one
    // lst1 - identifies the old-style filelistbox
    // lst2 - identifies the new-style filelistbox
    // see dlgs.h
    HWND flb = GetDlgItem(m_FileDialog,lst2);
    if (!IsWindow(flb))
        flb = GetDlgItem(m_FileDialog,lst1);

    return flb;
}


//##############################################################


//-----------------------------------------
// Special implementation for Win98
// because:
//
//-----------------------------------------

class CWin98PreviewAdapterImpl : public CPreviewAdapterImpl
{
public:
    CWin98PreviewAdapterImpl(HINSTANCE instance);

    virtual void SAL_CALL notifyParentWindowPosChanged();

protected:
    virtual void SAL_CALL rearrangeLayout();

    bool isValidToolbarDimension() const;

private:
    sal_Bool    m_PreviewActive;
    int         m_ToolbarPosX;
    int         m_ToolbarPosY;
    int         m_ToolbarWidth;
    int         m_ToolbarHeight;
};

//--------------------------------------------
//
//--------------------------------------------

CWin98PreviewAdapterImpl::CWin98PreviewAdapterImpl(HINSTANCE instance) :
    CPreviewAdapterImpl(instance),
    m_PreviewActive(sal_False),
    m_ToolbarPosX(0),
    m_ToolbarPosY(0),
    m_ToolbarWidth(0),
    m_ToolbarHeight(0)
{
}

//--------------------------------------------
//
//--------------------------------------------

void SAL_CALL CWin98PreviewAdapterImpl::notifyParentWindowPosChanged()
{
    try
    {
        // the reason for this condition is
        // Windows 98
        // Under Windows 98 the message WM_SHOWWINDOW
        // will be sent only the first time the
        // GetOpenFileName function is called within
        // the same process
        // so we must use another message to initialize
        // the preview window
        if (IsWindow(m_FileDialog) && !m_PreviewActive)
        {
            initializeActivePreview();
            m_PreviewActive = sal_True;
            rearrangeLayout();
        }

        if (IsWindow(m_FileDialog) && !isValidToolbarDimension())
        {
            RECT rcStc1;
            GetWindowRect(GetDlgItem(m_FileDialog,stc1),&rcStc1);

            RECT rcCmb2;
            GetWindowRect(GetDlgItem(m_FileDialog,cmb2),&rcCmb2);

            // Assumption:
            // the toolbar position is only valid
            // if the left edge is greater or equal
            // than the right edge of the drives listbox
            // the stc1 static text is invisible at runtime
            // but will be used as reference for the position
            // and dimension of the toolbar
            if (rcStc1.left >= rcCmb2.right)
            {
                // important: save the upper left corner in
                // client coordinates
                POINT pt = {rcStc1.left,rcStc1.top};
                ScreenToClient(m_FileDialog,&pt);

                m_ToolbarPosX   = pt.x;
                m_ToolbarPosY   = pt.y;
                m_ToolbarWidth  = rcStc1.right - rcStc1.left;
                m_ToolbarHeight = rcStc1.bottom - rcStc1.top;
            }
        }
    }
    catch(std::runtime_error&)
    {
    }
}

//--------------------------------------------
//
//--------------------------------------------

void SAL_CALL CWin98PreviewAdapterImpl::rearrangeLayout()
{
    CPreviewAdapterImpl::rearrangeLayout();

    // fix the position of the upper toolbar
    // because the FileDialog moves all windows
    // that are to the right of the FileListbox
    // so if we have changed the size of the
    // FileListbox we would run into trouble else
    if (isValidToolbarDimension())
    {
        HWND hwndTlb = FindWindowEx(
            m_FileDialog,NULL,TEXT("ToolbarWindow32"),NULL);

        SetWindowPos(hwndTlb,
            HWND_TOP,
            m_ToolbarPosX,
            m_ToolbarPosY,
            m_ToolbarWidth,
            m_ToolbarHeight,
            SWP_NOACTIVATE);
    }
}

//--------------------------------------------
//
//--------------------------------------------

bool CWin98PreviewAdapterImpl::isValidToolbarDimension() const
{
    return (m_ToolbarPosX   > 0 &&
            m_ToolbarPosY   > 0 &&
            m_ToolbarWidth  > 0 &&
            m_ToolbarHeight > 0);
}

//##############################################################


//--------------------------------------------
// Implementation for Windows 95/NT/ME/2000/XP
// because:
//
//--------------------------------------------

class CWin95NTPreviewAdapterImpl : public CPreviewAdapterImpl
{
public:
    CWin95NTPreviewAdapterImpl(HINSTANCE instance);

    virtual void SAL_CALL notifyParentShow(sal_Bool bShow);
};

//--------------------------------------------
//
//--------------------------------------------

CWin95NTPreviewAdapterImpl::CWin95NTPreviewAdapterImpl(HINSTANCE instance) :
    CPreviewAdapterImpl(instance)
{
}

//--------------------------------------------
//
//--------------------------------------------

void SAL_CALL CWin95NTPreviewAdapterImpl::notifyParentShow(sal_Bool bShow)
{
    try
    {
        if (bShow)
        {
            initializeActivePreview();
            rearrangeLayout();
        }
    }
    catch(std::runtime_error&)
    {
    }
}


//##############################################################


//-------------------------------
// ctor
//-------------------------------

CPreviewAdapter::CPreviewAdapter(HINSTANCE instance)
{
    if (!IsWindows98())
        m_pImpl.reset(new CWin95NTPreviewAdapterImpl(instance));
    else
        m_pImpl.reset(new CWin98PreviewAdapterImpl(instance));
}

//-------------------------------
//
//-------------------------------

CPreviewAdapter::~CPreviewAdapter()
{
}

//-------------------------------
//
//-------------------------------

Sequence<sal_Int16> SAL_CALL CPreviewAdapter::getSupportedImageFormats()
{
    com::sun::star::uno::Sequence<sal_Int16> imgFormats(1);
    imgFormats[0] = ::com::sun::star::ui::dialogs::FilePreviewImageFormats::BITMAP;
    return imgFormats;
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL CPreviewAdapter::getTargetColorDepth()
{
    return m_pImpl->getTargetColorDepth();
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL CPreviewAdapter::getAvailableWidth()
{
    return m_pImpl->getAvailableWidth();
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL CPreviewAdapter::getAvailableHeight()
{
    return m_pImpl->getAvailableHeight();
}

//-------------------------------
//
//-------------------------------

void SAL_CALL CPreviewAdapter::setImage( sal_Int16 aImageFormat, const Any& aImage )
    throw (IllegalArgumentException, RuntimeException)
{
    m_pImpl->setImage(aImageFormat,aImage);
}

//-------------------------------
//
//-------------------------------

sal_Bool SAL_CALL CPreviewAdapter::setShowState( sal_Bool bShowState )
{
    return m_pImpl->setShowState(bShowState);
}

//-------------------------------
//
//-------------------------------

sal_Bool SAL_CALL CPreviewAdapter::getShowState()
{
    return m_pImpl->getShowState();
}

//-------------------------------
//
//-------------------------------

void SAL_CALL CPreviewAdapter::setParent(HWND parent)
{
    m_pImpl->setParent(parent);
}

//-------------------------------
//
//-------------------------------

void SAL_CALL CPreviewAdapter::notifyParentShow(bool bShow)
{
    m_pImpl->notifyParentShow(bShow);
}

//-------------------------------
//
//-------------------------------

void SAL_CALL CPreviewAdapter::notifyParentSizeChanged()
{
    m_pImpl->notifyParentSizeChanged();
}

//-------------------------------
//
//-------------------------------

void SAL_CALL CPreviewAdapter::notifyParentWindowPosChanged()
{
    m_pImpl->notifyParentWindowPosChanged();
}
