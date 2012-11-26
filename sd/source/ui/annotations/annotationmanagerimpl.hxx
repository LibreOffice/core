/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    //
    void ExecuteAnnotation (SfxRequest& rRequest);
    void GetAnnotationState (SfxItemSet& rItemSet);

    void ExecuteInsertAnnotation(SfxRequest& rReq);
    void ExecuteDeleteAnnotation(SfxRequest& rReq);
    void ExecuteReplyToAnnotation(SfxRequest& rReq);

    void SelectNextAnnotation(bool bForeward);

    void SelectAnnotation( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > xAnnotation, bool bEdit = false );
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
#if 0
    rtl::OUString GetHelpText( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation );
#endif
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
