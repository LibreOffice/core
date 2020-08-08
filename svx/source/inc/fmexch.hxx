/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_SOURCE_INC_FMEXCH_HXX
#define INCLUDED_SVX_SOURCE_INC_FMEXCH_HXX

#include <config_options.h>
#include <sal/config.h>

#include <set>

#include <sot/exchange.hxx>
#include <vcl/transfer.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/form/XForms.hpp>
#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>
#include <svx/svxdllapi.h>

namespace svxform
{
    typedef ::std::set<std::unique_ptr<weld::TreeIter>> ListBoxEntrySet;

    //= OLocalExchange
    class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) OLocalExchange : public TransferDataContainer
    {
    private:
        Link<OLocalExchange&,void>  m_aClipboardListener;
        bool            m_bDragging         : 1;
        bool            m_bClipboardOwner   : 1;

    public:
        class GrantAccess
        {
            friend class OLocalExchangeHelper;
        };

    public:
        OLocalExchange( );

        bool        isDragging() const { return m_bDragging; }
        bool        isClipboardOwner() const { return m_bClipboardOwner; }

        void        setDragging(bool bDragging);
        void        copyToClipboard(const GrantAccess&);

        void        setClipboardListener( const Link<OLocalExchange&,void>& _rListener ) { m_aClipboardListener = _rListener; }

        void        clear();

        static  bool    hasFormat( const DataFlavorExVector& _rFormats, SotClipboardFormatId _nFormatId );

    protected:
        // XClipboardOwner
        virtual void SAL_CALL lostOwnership( const css::uno::Reference< css::datatransfer::clipboard::XClipboard >& _rxClipboard, const css::uno::Reference< css::datatransfer::XTransferable >& _rxTrans ) override;

        // TransferableHelper
        virtual void        DragFinished( sal_Int8 nDropAction ) override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;

    private:
         // don't allow this base class method to be called from outside
        using TransferDataContainer::StartDrag;
    };


    //= OLocalExchangeHelper

    /// a helper for navigator windows (SvTreeListBox'es) which allow DnD within themself
    class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) OLocalExchangeHelper
    {
    protected:
        rtl::Reference<OLocalExchange> m_xTransferable;

    public:
        OLocalExchangeHelper();
        virtual ~OLocalExchangeHelper();

        void        prepareDrag( );

        void        copyToClipboard( ) const;

        bool    isDragSource() const { return m_xTransferable.is() && m_xTransferable->isDragging(); }
        bool    isClipboardOwner() const { return m_xTransferable.is() && m_xTransferable->isClipboardOwner(); }
        bool    isDataExchangeActive( ) const { return isDragSource() || isClipboardOwner(); }
        void        clear() { if ( isDataExchangeActive() ) m_xTransferable->clear(); }

        SVX_DLLPRIVATE void     setClipboardListener( const Link<OLocalExchange&,void>& _rListener ) { if ( m_xTransferable.is() ) m_xTransferable->setClipboardListener( _rListener ); }

    protected:
        SVX_DLLPRIVATE virtual OLocalExchange* createExchange() const = 0;

    protected:
        SVX_DLLPRIVATE void implReset();
    };

    class OControlTransferData
    {
    private:
        DataFlavorExVector  m_aCurrentFormats;

    protected:
        ListBoxEntrySet     m_aSelectedEntries;
        css::uno::Sequence< css::uno::Sequence< sal_uInt32 > >
                            m_aControlPaths;
        css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >
                            m_aHiddenControlModels;

        css::uno::Reference< css::form::XForms >
                            m_xFormsRoot;       // the root of the forms collection where the entries we represent reside
                                                // this uniquely identifies the page and the document

        bool        m_bFocusEntry;

    protected:
        // updates m_aCurrentFormats with all formats we currently could supply
        void    updateFormats( );

    public:
        OControlTransferData( );

        // ctor to construct the data from an arbitrary Transferable (usually clipboard data)
        OControlTransferData(
            const css::uno::Reference< css::datatransfer::XTransferable >& _rxTransferable
        );

        inline const DataFlavorExVector&    GetDataFlavorExVector() const;

        void addSelectedEntry(std::unique_ptr<weld::TreeIter> xEntry);
        void setFocusEntry(bool _bFocusEntry);

        /** notifies the data transfer object that a certain entry has been removed from the owning tree

            In case the removed entry is part of the transfer object's selection, the entry is removed from
            the selection.

            @param  _pEntry
            @return the number of entries remaining in the selection.
        */
        size_t  onEntryRemoved(weld::TreeView* pView, weld::TreeIter* _pEntry);

        void setFormsRoot(
            const css::uno::Reference< css::form::XForms >& _rxFormsRoot
            ) { m_xFormsRoot = _rxFormsRoot; }

        void buildPathFormat(const weld::TreeView* pTreeBox, const weld::TreeIter* pRoot);
            // assembles m_aControlPaths from m_aSelectedEntries
            // (it is assumed that the entries are sorted in m_aSelectedEntries with respect to the neighbor relationship)


        void buildListFromPath(const weld::TreeView* pTreeBox, weld::TreeIter* pRoot);
            // The reverse way: throws everything out of m_aSelectedEntries and rebuilds it using m_aControlPaths

        void addHiddenControlsFormat(const css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >& seqInterfaces);
            // adds an SVX_FML_HIDDEN_CONTROLS format and remembers the passed interfaces for it
            // (it is NOT checked whether actually only hidden controls are denominated
            // by this - the caller must ensure that)

        const ListBoxEntrySet&      selected() const { return m_aSelectedEntries; }
        const css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >&
                                    hiddenControls() const { return m_aHiddenControlModels; }

        const css::uno::Reference< css::form::XForms >&
                                getFormsRoot() const { return m_xFormsRoot; }
    };


    inline const DataFlavorExVector& OControlTransferData::GetDataFlavorExVector() const
    {
        const_cast< OControlTransferData* >( this )->updateFormats( );
        return m_aCurrentFormats;
    }

    class OControlExchange : public OLocalExchange, public OControlTransferData
    {
    public:
        OControlExchange( );

    public:
        static SotClipboardFormatId getFieldExchangeFormatId( );
        static SotClipboardFormatId getControlPathFormatId( );
        static SotClipboardFormatId getHiddenControlModelsFormatId( );

        inline static bool  hasFieldExchangeFormat( const DataFlavorExVector& _rFormats );
        inline static bool  hasControlPathFormat( const DataFlavorExVector& _rFormats );
        inline static bool  hasHiddenControlModelsFormat( const DataFlavorExVector& _rFormats );

    protected:
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        virtual void        AddSupportedFormats() override;
    };

    class OControlExchangeHelper : public OLocalExchangeHelper
    {
    public:
        OControlExchangeHelper() : OLocalExchangeHelper() { }

        OControlExchange* operator->() const { return static_cast< OControlExchange* >( m_xTransferable.get() ); }
        OControlExchange& operator*() const { return *static_cast< OControlExchange* >( m_xTransferable.get() ); }

    protected:
        virtual OLocalExchange* createExchange() const override;
    };


    inline bool OControlExchange::hasFieldExchangeFormat( const DataFlavorExVector& _rFormats )
    {
        return hasFormat( _rFormats, getFieldExchangeFormatId() );
    }

    inline bool OControlExchange::hasControlPathFormat( const DataFlavorExVector& _rFormats )
    {
        return hasFormat( _rFormats, getControlPathFormatId() );
    }

    inline bool OControlExchange::hasHiddenControlModelsFormat( const DataFlavorExVector& _rFormats )
    {
        return hasFormat( _rFormats, getHiddenControlModelsFormatId() );
    }


}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
