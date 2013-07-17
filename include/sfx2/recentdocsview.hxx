/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_RECENTDOCSVIEW_HXX__
#define __SFX2_RECENTDOCSVIEW_HXX__


#include <sfx2/thumbnailview.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <cppuhelper/compbase1.hxx>

class RecentDocViewItem : public ThumbnailViewItem
{
public:
    RecentDocViewItem(ThumbnailView &rView)
        : ThumbnailViewItem(rView)
    {
    }
    OUString aURL;
    OUString aTitle;
};

struct LoadRecentFile
{
    ::com::sun::star::util::URL                                                 aTargetURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   aArgSeq;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >      xDispatch;
};

typedef ::cppu::WeakComponentImplHelper1<
    com::sun::star::lang::XInitialization> RecentDocsViewType;

class SFX2_DLLPUBLIC RecentDocsView :   protected ::comphelper::OBaseMutex,
                                        public RecentDocsViewType,
                                        public ThumbnailView
{
public:
    RecentDocsView( Window* pParent );
    virtual ~RecentDocsView();

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    void insertItem(const OUString &rURL, const OUString &rTitle);
    void loadRecentDocs();

    DECL_STATIC_LINK( RecentDocsView, ExecuteHdl_Impl, LoadRecentFile* );

protected:
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem);

private:
    bool                                                                            m_bInitialized;
    OUString                                                                        m_aModuleName;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >             m_xFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >     m_xURLTransformer;
};

#endif  // __SFX2_RECENTDOCSVIEW_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
