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

#include <fmexch.hxx>

#include <sot/formats.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace svxform
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::datatransfer;

    OLocalExchange::OLocalExchange( )
        :m_bDragging( false )
        ,m_bClipboardOwner( false )
    {
    }

    void OLocalExchange::copyToClipboard(const weld::Widget& rWidget, const GrantAccess&)
    {
        if ( m_bClipboardOwner )
        {   // simulate a lostOwnership to notify parties interested in
            m_aClipboardListener.Call( *this );
        }

        m_bClipboardOwner = true;
        CopyToClipboard(rWidget.get_clipboard());
    }

    void OLocalExchange::clear()
    {
        if ( !isClipboardOwner() )
            return;

        try
        {
            Reference< clipboard::XClipboard > xClipBoard( getOwnClipboard() );
            if ( xClipBoard.is() )
                xClipBoard->setContents( nullptr, nullptr );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
        m_bClipboardOwner = false;
    }

    void SAL_CALL OLocalExchange::lostOwnership( const Reference< clipboard::XClipboard >& _rxClipboard, const Reference< XTransferable >& _rxTrans )
    {
        TransferDataContainer::implCallOwnLostOwnership( _rxClipboard, _rxTrans );
        m_bClipboardOwner = false;

        m_aClipboardListener.Call( *this );
    }

    void OLocalExchange::setDragging(bool bDragging)
    {
        m_bDragging = bDragging;
    }

    void OLocalExchange::DragFinished( sal_Int8 nDropAction )
    {
        TransferDataContainer::DragFinished( nDropAction );
        setDragging(false);
    }

    bool OLocalExchange::hasFormat( const DataFlavorExVector& _rFormats, SotClipboardFormatId _nFormatId )
    {
        return std::any_of(_rFormats.begin(), _rFormats.end(),
            [&_nFormatId](const DataFlavorEx& rFormat) { return rFormat.mnSotId == _nFormatId; });
    }

    bool OLocalExchange::GetData( const css::datatransfer::DataFlavor& /*_rFlavor*/, const OUString& /*rDestDoc*/ )
    {
        return false;   // do not have any formats by default
    }

    OControlTransferData::OControlTransferData( )
        : m_bFocusEntry(false)
    {
    }

    OControlTransferData::OControlTransferData( const Reference< XTransferable >& _rxTransferable )
        : m_bFocusEntry(false)
    {
        TransferableDataHelper aExchangedData( _rxTransferable );

        // try the formats we know
        if ( OControlExchange::hasControlPathFormat( aExchangedData.GetDataFlavorExVector() ) )
        {   // paths to the controls, relative to a root
            Sequence< Any > aControlPathData;
            if ( aExchangedData.GetAny(OControlExchange::getControlPathFormatId(), OUString()) >>= aControlPathData )
            {
                DBG_ASSERT( aControlPathData.getLength() >= 2, "OControlTransferData::OControlTransferData: invalid data for the control path format!" );
                if ( aControlPathData.getLength() >= 2 )
                {
                    aControlPathData[0] >>= m_xFormsRoot;
                    aControlPathData[1] >>= m_aControlPaths;
                }
            }
            else
            {
                OSL_FAIL( "OControlTransferData::OControlTransferData: invalid data for the control path format (2)!" );
            }
        }
        if ( OControlExchange::hasHiddenControlModelsFormat( aExchangedData.GetDataFlavorExVector() ) )
        {   // sequence of models of hidden controls
            aExchangedData.GetAny(OControlExchange::getHiddenControlModelsFormatId(), OUString()) >>= m_aHiddenControlModels;
        }

        updateFormats( );
    }


    static bool lcl_fillDataFlavorEx( SotClipboardFormatId nId, DataFlavorEx& _rFlavor )
    {
        _rFlavor.mnSotId = nId;
        return SotExchange::GetFormatDataFlavor( _rFlavor.mnSotId, _rFlavor );
    }


    void OControlTransferData::updateFormats( )
    {
        m_aCurrentFormats.clear();
        m_aCurrentFormats.reserve( 3 );

        DataFlavorEx aFlavor;

        if ( m_aHiddenControlModels.hasElements() )
        {
            if ( lcl_fillDataFlavorEx( OControlExchange::getHiddenControlModelsFormatId(), aFlavor ) )
                m_aCurrentFormats.push_back( aFlavor );
        }

        if ( m_xFormsRoot.is() && m_aControlPaths.hasElements() )
        {
            if ( lcl_fillDataFlavorEx( OControlExchange::getControlPathFormatId(), aFlavor ) )
                m_aCurrentFormats.push_back( aFlavor );
        }

        if ( !m_aSelectedEntries.empty() )
        {
            if ( lcl_fillDataFlavorEx( OControlExchange::getFieldExchangeFormatId(), aFlavor ) )
                m_aCurrentFormats.push_back( aFlavor );
        }
    }

    size_t OControlTransferData::onEntryRemoved(const weld::TreeView* pView, const weld::TreeIter* _pEntry)
    {
        auto aIter = std::find_if(m_aSelectedEntries.begin(), m_aSelectedEntries.end(),
                                  [pView, _pEntry](const auto& rElem) {
                                    return pView->iter_compare(*rElem, *_pEntry) == 0;
                                  });
        if (aIter != m_aSelectedEntries.end())
            m_aSelectedEntries.erase(aIter);

        return m_aSelectedEntries.size();
    }

    void OControlTransferData::addSelectedEntry(std::unique_ptr<weld::TreeIter> xEntry)
    {
        m_aSelectedEntries.emplace(std::move(xEntry));
    }

    void OControlTransferData::setFocusEntry(bool _bFocusEntry)
    {
        m_bFocusEntry = _bFocusEntry;
    }

    void OControlTransferData::addHiddenControlsFormat(const css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >& seqInterfaces)
    {
        m_aHiddenControlModels = seqInterfaces;
    }

    void OControlTransferData::buildPathFormat(const weld::TreeView* pTreeBox, const weld::TreeIter* pRoot)
    {
        m_aControlPaths.realloc(0);

        sal_Int32 nEntryCount = m_aSelectedEntries.size();
        if (nEntryCount == 0)
            return;

        m_aControlPaths.realloc(nEntryCount);
        css::uno::Sequence<sal_uInt32>* pAllPaths = m_aControlPaths.getArray();
        for (const auto& rCurrentEntry : m_aSelectedEntries)
        {
            // first we collect the path in an array
            ::std::vector< sal_uInt32 > aCurrentPath;

            std::unique_ptr<weld::TreeIter> xLoop(pTreeBox->make_iterator(rCurrentEntry.get()));
            while (pTreeBox->iter_compare(*xLoop, *pRoot) != 0)
            {
                aCurrentPath.push_back(pTreeBox->get_iter_index_in_parent(*xLoop));
                bool bLoop = pTreeBox->iter_parent(*xLoop);
                assert(bLoop && "OControlTransferData::buildPathFormat: invalid root or entry !"); (void)bLoop;
            }

            // then we can transfer it into css::uno::Sequence
            Sequence<sal_uInt32>& rCurrentPath = *pAllPaths;
            sal_Int32 nDepth = aCurrentPath.size();

            rCurrentPath.realloc(nDepth);
            sal_uInt32* pSeq = rCurrentPath.getArray();
            sal_Int32 j,k;
            for (j = nDepth - 1, k = 0; k<nDepth; --j, ++k)
                pSeq[j] = aCurrentPath[k];
            ++pAllPaths;
        }
    }

    void OControlTransferData::buildListFromPath(const weld::TreeView* pTreeBox, const weld::TreeIter* pRoot)
    {
        ListBoxEntrySet().swap(m_aSelectedEntries);

        for (const css::uno::Sequence<sal_uInt32>& rPaths : m_aControlPaths)
        {
            std::unique_ptr<weld::TreeIter> xSearch(pTreeBox->make_iterator(pRoot));
            for (const sal_uInt32 nThisPath : rPaths)
                pTreeBox->iter_nth_child(*xSearch, nThisPath);
            m_aSelectedEntries.emplace(std::move(xSearch));
        }
    }

    OControlExchange::OControlExchange( )
    {
    }

    bool OControlExchange::GetData( const DataFlavor& _rFlavor, const OUString& rDestDoc )
    {
        const SotClipboardFormatId nFormatId = SotExchange::GetFormat( _rFlavor );

        if ( getControlPathFormatId( ) == nFormatId )
        {
            // ugly. We have to pack all the info into one object
            Sequence< Any > aCompleteInfo( 2 );
            OSL_ENSURE( m_xFormsRoot.is(), "OLocalExchange::GetData: invalid forms root for this format!" );
            aCompleteInfo.getArray()[ 0 ] <<= m_xFormsRoot;
            aCompleteInfo.getArray()[ 1 ] <<= m_aControlPaths;

            SetAny( Any( aCompleteInfo ) );
        }
        else if ( getHiddenControlModelsFormatId() == nFormatId )
        {
            // just need to transfer the models
            SetAny( Any( m_aHiddenControlModels ) );
        }
        else
            return OLocalExchange::GetData(_rFlavor, rDestDoc);

        return true;
    }

    void OControlExchange::AddSupportedFormats()
    {
        if (m_bFocusEntry && !m_aSelectedEntries.empty())
            AddFormat(getFieldExchangeFormatId());

        if (m_aControlPaths.hasElements())
            AddFormat(getControlPathFormatId());

        if (m_aHiddenControlModels.hasElements())
            AddFormat(getHiddenControlModelsFormatId());
    }

    SotClipboardFormatId OControlExchange::getControlPathFormatId()
    {
        static SotClipboardFormatId s_nFormat =
            SotExchange::RegisterFormatName(u"application/x-openoffice;windows_formatname=\"svxform.ControlPathExchange\""_ustr);
        DBG_ASSERT(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OControlExchange::getControlPathFormatId: bad exchange id!");
        return s_nFormat;
    }

    SotClipboardFormatId OControlExchange::getHiddenControlModelsFormatId()
    {
        static SotClipboardFormatId s_nFormat =
            SotExchange::RegisterFormatName(u"application/x-openoffice;windows_formatname=\"svxform.HiddenControlModelsExchange\""_ustr);
        DBG_ASSERT(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OControlExchange::getHiddenControlModelsFormatId: bad exchange id!");
        return s_nFormat;
    }


    SotClipboardFormatId OControlExchange::getFieldExchangeFormatId()
    {
        static SotClipboardFormatId s_nFormat =
            SotExchange::RegisterFormatName(u"application/x-openoffice;windows_formatname=\"svxform.FieldNameExchange\""_ustr);
        DBG_ASSERT(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OControlExchange::getFieldExchangeFormatId: bad exchange id!");
        return s_nFormat;
    }

    //= OControlExchangeHelper
    rtl::Reference<OLocalExchange> OControlExchangeHelper::createExchange() const
    {
        return new OControlExchange;
    }

    OLocalExchangeHelper::OLocalExchangeHelper()
    {
    }

    OLocalExchangeHelper::~OLocalExchangeHelper()
    {
        implReset();
    }

    void OLocalExchangeHelper::copyToClipboard(const weld::Widget& rWidget) const
    {
        DBG_ASSERT( m_xTransferable.is(), "OLocalExchangeHelper::copyToClipboard: not prepared!" );
        m_xTransferable->copyToClipboard(rWidget, OLocalExchange::GrantAccess());
    }

    void OLocalExchangeHelper::implReset()
    {
        if (m_xTransferable.is())
        {
            m_xTransferable->setClipboardListener( Link<OLocalExchange&,void>() );
            m_xTransferable.clear();
        }
    }

    void OLocalExchangeHelper::prepareDrag( )
    {
        DBG_ASSERT(!m_xTransferable.is() || !m_xTransferable->isDragging(), "OLocalExchangeHelper::prepareDrag: recursive DnD?");

        implReset();
        m_xTransferable = createExchange();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
