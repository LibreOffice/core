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

#ifndef _DBAUI_INDEXDIALOG_HXX_
#define _DBAUI_INDEXDIALOG_HXX_

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/treelistbox.hxx>
#include <unotools/viewoptions.hxx>
#include "indexes.hxx"
#include <dbaccess/ToolBoxHelper.hxx>

namespace dbaui
{

    // DbaIndexList
    class DbaIndexList : public SvTreeListBox
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
        Link        m_aSelectHdl;
        Link        m_aEndEditHdl;
        sal_Bool    m_bSuspendSelectHdl;

    public:
        DbaIndexList(Window* _pParent, const ResId& _rId);

        void SetSelectHdl(const Link& _rHdl) { m_aSelectHdl = _rHdl; }
        Link GetSelectHdl() const { return m_aSelectHdl; }

        void SetEndEditHdl(const Link& _rHdl) { m_aEndEditHdl = _rHdl; }
        Link GetEndEditHdl() const { return m_aEndEditHdl; }

        virtual sal_Bool Select( SvTreeListEntry* pEntry, sal_Bool bSelect );

        void enableSelectHandler();
        void disableSelectHandler();

        void SelectNoHandlerCall( SvTreeListEntry* pEntry );

        inline void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection)
        {
             m_xConnection = _rxConnection;
        }

    protected:
        virtual sal_Bool EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText );

    private:
        using SvTreeListBox::Select;
    };

    // DbaIndexDialog
    class IndexFieldsControl;
    class OIndexCollection;
    class DbaIndexDialog :  public ModalDialog,
                            public OToolBoxHelper
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
        SvtViewOptions          m_aGeometrySettings;

        ToolBox                 m_aActions;
        DbaIndexList            m_aIndexes;
        FixedLine               m_aIndexDetails;
        FixedText               m_aDescriptionLabel;
        FixedText               m_aDescription;
        CheckBox                m_aUnique;
        FixedText               m_aFieldsLabel;
        IndexFieldsControl*     m_pFields;
        PushButton              m_aClose;
        HelpButton              m_aHelp;

        OIndexCollection*       m_pIndexes;
        SvTreeListEntry*            m_pPreviousSelection;
        sal_Bool                m_bEditAgain;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                m_xContext;
    public:
        DbaIndexDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Sequence< OUString >& _rFieldNames,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            sal_Int32 _nMaxColumnsInIndex
            );
        virtual ~DbaIndexDialog();

        virtual void StateChanged( StateChangedType nStateChange );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

        /** will be called whenthe id of the image list is needed.
            @param  _eBitmapSet
                <svtools/imgdef.hxx>
            @param  _bHiContast
                <TRUE/> when in high contrast mode.
        */
        virtual ImageList getImageList(sal_Int16 _eBitmapSet) const;

        /** will be called when the controls need to be resized.
        */
        virtual void resizeControls(const Size& _rDiff);

    protected:
        void fillIndexList();
        void updateToolbox();
        void updateControls(const SvTreeListEntry* _pEntry);

    protected:
        DECL_LINK( OnIndexSelected, DbaIndexList* );
        DECL_LINK( OnIndexAction, ToolBox* );
        DECL_LINK( OnEntryEdited, SvTreeListEntry* );
        DECL_LINK( OnModified, void* );
        DECL_LINK( OnCloseDialog, void* );

        DECL_LINK( OnEditIndexAgain, SvTreeListEntry* );

    private:
        void OnNewIndex();
        void OnDropIndex(sal_Bool _bConfirm = sal_True);
        void OnRenameIndex();
        void OnSaveIndex();
        void OnResetIndex();

        sal_Bool implCommit(SvTreeListEntry* _pEntry);
        sal_Bool implSaveModified(sal_Bool _bPlausibility = sal_True);
        sal_Bool implCommitPreviouslySelected();

        sal_Bool implDropIndex(SvTreeListEntry* _pEntry, sal_Bool _bRemoveFromCollection);

        sal_Bool implCheckPlausibility(const ConstIndexesIterator& _rPos);

        /** checks if the controls have to be replaced and moved.
        */
        void checkControls();
    };

}   // namespace dbaui

#endif // _DBAUI_INDEXDIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
