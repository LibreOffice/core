/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SD_ANNOTATIONMANAGER_IMPL_HXX
#define _SD_ANNOTATIONMANAGER_IMPL_HXX

#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>

#include <rtl/ustring.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>

#include "ViewShellBase.hxx"

#include "annotationtag.hxx"

class SfxRequest;

namespace sd
{

typedef std::vector< rtl::Reference< AnnotationTag > > AnnotationTagVector;

namespace tools {
class EventMultiplexerEvent;
}

typedef ::cppu::WeakComponentImplHelper1 <
    com::sun::star::document::XEventListener
    > AnnotationManagerImplBase;

class AnnotationManagerImpl : private ::cppu::BaseMutex, public AnnotationManagerImplBase
{
public:
    AnnotationManagerImpl( ViewShellBase& rViewShellBase );

    void init();

    // WeakComponentImplHelper1
    virtual void SAL_CALL disposing ();

    // XEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    void ExecuteAnnotation (SfxRequest& rRequest);
    void GetAnnotationState (SfxItemSet& rItemSet);

    void ExecuteInsertAnnotation(SfxRequest& rReq);
    void ExecuteDeleteAnnotation(SfxRequest& rReq);
    void ExecuteReplyToAnnotation(SfxRequest& rReq);

    void SelectNextAnnotation(bool bForeward);

    void SelectAnnotation( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > xAnnotation, bool bEdit = sal_False );
    void GetSelectedAnnotation( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation );

    void InsertAnnotation();
    void DeleteAnnotation( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > xAnnotation );
    void DeleteAnnotationsByAuthor( const rtl::OUString& sAuthor );
    void DeleteAllAnnotations();

    void ExecuteAnnotationContextMenu( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > xAnnotation, ::Window* pParent, const Rectangle& rContextRect, bool bButtonMenu = false );

    Color GetColorDark(sal_uInt16 aAuthorIndex);
    Color GetColorLight(sal_uInt16 aAuthorIndex);
    Color GetColor(sal_uInt16 aAuthorIndex);


    // callbacks
    void onTagSelected( AnnotationTag& rTag );
    void onTagDeselected( AnnotationTag& rTag );

    void onSelectionChanged();

    void addListener();
    void removeListener();

    void invalidateSlots();

    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
    DECL_LINK(UpdateTagsHdl, void *);

    void UpdateTags(bool bSynchron = false);
    void CreateTags();
    void DisposeTags();

    SdPage* GetNextPage( SdPage* pPage, bool bForeward );
    SdPage* GetFirstPage();
    SdPage* GetLastPage();

    SdPage* GetCurrentPage();

    SdDrawDocument* GetDoc() { return mpDoc; }

    void ShowAnnotations(bool bShow);

private:
    ViewShellBase& mrBase;
    SdDrawDocument* mpDoc;

    AnnotationTagVector maTagVector;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView > mxView;
    ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotationAccess > mxCurrentPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > mxSelectedAnnotation;

    bool mbShowAnnotations;
    sal_uLong mnUpdateTagsEvent;
    Font maFont;
};

}

#endif // _SD_ANNOTATIONMANAGER_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
