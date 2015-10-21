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

#include <sal/config.h>

#include <set>

#include <sot/exchange.hxx>
#include <svtools/transfer.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/form/XForms.hpp>
#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <svx/svxdllapi.h>

class SvTreeListEntry;
class SvTreeListBox;

namespace svxform
{




    typedef ::std::set< SvTreeListEntry* >  ListBoxEntrySet;


    //= OLocalExchange

    class SVX_DLLPUBLIC OLocalExchange : public TransferableHelper
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

        void        startDrag( vcl::Window* pWindow, sal_Int8 nDragSourceActions, const GrantAccess& );
        void        copyToClipboard( vcl::Window* _pWindow, const GrantAccess& );

        void        setClipboardListener( const Link<OLocalExchange&,void>& _rListener ) { m_aClipboardListener = _rListener; }

        void        clear();

        static  bool    hasFormat( const DataFlavorExVector& _rFormats, SotClipboardFormatId _nFormatId );

    protected:
        // XClipboardOwner
        virtual void SAL_CALL lostOwnership( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& _rxClipboard, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& _rxTrans ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // TransferableHelper
        virtual void        DragFinished( sal_Int8 nDropAction ) override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;

    private:
        void StartDrag( vcl::Window* pWindow, sal_Int8 nDragSourceActions, sal_Int32 nDragPointer = DND_POINTER_NONE, sal_Int32 nDragImage = DND_IMAGE_NONE )
        {   // don't allow this base class method to be called from outside
            TransferableHelper::StartDrag(pWindow, nDragSourceActions, nDragPointer, nDragImage);
        }
    };


    //= OLocalExchangeHelper

    /// a helper for navigator windows (SvTreeListBox'es) which allow DnD within themself
    class SVX_DLLPUBLIC OLocalExchangeHelper
    {
    protected:
        VclPtr<vcl::Window> m_pDragSource;
        OLocalExchange*     m_pTransferable;

    public:
        OLocalExchangeHelper( vcl::Window* _pDragSource );
        virtual ~OLocalExchangeHelper();

        void        prepareDrag( );

        void        startDrag( sal_Int8 nDragSourceActions );
        void        copyToClipboard( ) const;

        inline  bool    isDragSource() const { return m_pTransferable && m_pTransferable->isDragging(); }
        inline  bool    isClipboardOwner() const { return m_pTransferable && m_pTransferable->isClipboardOwner(); }
        inline  bool    isDataExchangeActive( ) const { return isDragSource() || isClipboardOwner(); }
        inline  void        clear() { if ( isDataExchangeActive() ) m_pTransferable->clear(); }

        SVX_DLLPRIVATE void     setClipboardListener( const Link<OLocalExchange&,void>& _rListener ) { if ( m_pTransferable ) m_pTransferable->setClipboardListener( _rListener ); }

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
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
                            m_aHiddenControlModels;

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForms >
                            m_xFormsRoot;       // the root of the forms collection where the entries we represent reside
                                                // this uniquely identifies the page and the document

        SvTreeListEntry*        m_pFocusEntry;

    protected:
        // updates m_aCurrentFormats with all formats we currently could supply
        void    updateFormats( );

    public:
        OControlTransferData( );

        // ctor to construct the data from an arbitrary Transferable (usually clipboard data)
        OControlTransferData(
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& _rxTransferable
        );

        inline const DataFlavorExVector&    GetDataFlavorExVector() const;

        void addSelectedEntry( SvTreeListEntry* _pEntry );
        void setFocusEntry( SvTreeListEntry* _pFocusEntry );

        /** notifies the data transfer object that a certain entry has been removed from the owning tree

            In case the removed entry is part of the transfer object's selection, the entry is removed from
            the selection.

            @param  _pEntry
            @return the number of entries remaining in the selection.
        */
        size_t  onEntryRemoved( SvTreeListEntry* _pEntry );

        void setFormsRoot(
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForms >& _rxFormsRoot
            ) { m_xFormsRoot = _rxFormsRoot; }

        void buildPathFormat(SvTreeListBox* pTreeBox, SvTreeListEntry* pRoot);
            // baut aus m_aSelectedEntries m_aControlPaths auf
            // (es wird davon ausgegangen, dass die Eintraege in m_aSelectedEntries sortiert sind in Bezug auf die Nachbar-Beziehung)


        void buildListFromPath(SvTreeListBox* pTreeBox, SvTreeListEntry* pRoot);
            // der umgekehrte Weg : wirft alles aus m_aSelectedEntries weg und baut es mittels m_aControlPaths neu auf

        void addHiddenControlsFormat(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& seqInterfaces);
            // fuegt ein SVX_FML_HIDDEN_CONTROLS-Format hinzu und merk sich dafuer die uebergebenen Interfaces
            // (es erfolgt KEINE Ueberpruefung, ob dadurch auch tatsaechlich nur hidden Controls bezeichnet werden, dass muss der
            // Aufrufer sicherstellen)

        const ListBoxEntrySet&      selected() const { return m_aSelectedEntries; }
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
                                    hiddenControls() const { return m_aHiddenControlModels; }

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForms >
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
        OControlExchangeHelper(vcl::Window* _pDragSource) : OLocalExchangeHelper(_pDragSource) { }

        OControlExchange* operator->() const { return static_cast< OControlExchange* >( m_pTransferable ); }
        OControlExchange& operator*() const { return *static_cast< OControlExchange* >( m_pTransferable ); }

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
