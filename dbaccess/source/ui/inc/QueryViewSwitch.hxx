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
#ifndef DBAUI_QUERYVIEWSWITCH_HXX
#define DBAUI_QUERYVIEWSWITCH_HXX

#include "queryview.hxx"

namespace dbtools
{
    class SQLExceptionInfo;
}

namespace dbaui
{
    class OQueryDesignView;
    class OQueryTextView;
    class OAddTableDlg;
    class OQueryContainerWindow;
    class OQueryViewSwitch
    {
        OQueryDesignView*   m_pDesignView;
        OQueryTextView*     m_pTextView;
        sal_Bool            m_bAddTableDialogWasVisible; // true if so
    public:
        OQueryViewSwitch(OQueryContainerWindow* pParent, OQueryController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
        virtual ~OQueryViewSwitch();

        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        // clears the whole query
        virtual void clear();
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);
        // check if the statement is correct when not returning false
        virtual sal_Bool checkStatement();
        // set the statement for representation
        virtual void setStatement(const OUString& _rsStatement);
        // returns the current sql statement
        virtual OUString getStatement();
        /// late construction
        virtual void Construct();
        virtual void initialize();
        /** show the text or the design view
            @return
                <TRUE/> if and only if the view could be successfully, switched, <FALSE/> otherwise
                (In the latter case, the controller will issue another switchView call to restore the
                old state)
        */
        bool     switchView( ::dbtools::SQLExceptionInfo* _pErrorInfo );
        void     forceInitialView();
        sal_Bool isSlotEnabled(sal_Int32 _nSlotId);
        void     setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable);
        void     setNoneVisbleRow(sal_Int32 _nRows);
        void     SaveUIConfig();
        bool     reset( ::dbtools::SQLExceptionInfo* _pErrorInfo );
        void     GrabFocus();

        // returs the add table dialog from the design view
        OAddTableDlg* getAddTableDialog();

        OQueryDesignView*       getDesignView() const { return m_pDesignView; }
        OQueryContainerWindow*  getContainer() const;

        void SetPosSizePixel( Point _rPt,Size _rSize);
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getORB() const;
    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);

    private:
        void    impl_forceSQLView();
        bool    impl_postViewSwitch( const bool i_bGraphicalDesign, const bool i_bSuccess );
    };
}
#endif // DBAUI_QUERYVIEWSWITCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
