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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYVIEWSWITCH_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYVIEWSWITCH_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>

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
    class OQueryController;

    class OQueryViewSwitch final
    {
        VclPtr<OQueryDesignView>   m_pDesignView;
        VclPtr<OQueryTextView>     m_pTextView;
        bool            m_bAddTableDialogWasVisible; // true if so
    public:
        OQueryViewSwitch(OQueryContainerWindow* pParent, OQueryController& _rController,const css::uno::Reference< css::uno::XComponentContext >& );
        ~OQueryViewSwitch();

        bool isCutAllowed() const;
        bool isPasteAllowed() const;
        bool isCopyAllowed() const;
        void copy();
        void cut();
        void paste();
        // clears the whole query
        void clear();
        // check if the statement is correct when not returning false
        bool checkStatement();
        // set the statement for representation
        void setStatement(const OUString& _rsStatement);
        // returns the current sql statement
        OUString getStatement();
        /// late construction
        void Construct();
        void initialize();
        /** show the text or the design view
            @return
                <TRUE/> if and only if the view could be successfully, switched, <FALSE/> otherwise
                (In the latter case, the controller will issue another switchView call to restore the
                old state)
        */
        bool     switchView( ::dbtools::SQLExceptionInfo* _pErrorInfo );
        void     forceInitialView();
        bool     isSlotEnabled(sal_Int32 _nSlotId);
        void     setSlotEnabled(sal_Int32 _nSlotId, bool _bEnable);
        void     setNoneVisibleRow(sal_Int32 _nRows);
        void     SaveUIConfig();
        void     reset();
        void     GrabFocus();

        // returns the add table dialog from the design view
        OAddTableDlg* getAddTableDialog();

        OQueryDesignView*       getDesignView() const { return m_pDesignView; }
        OQueryContainerWindow*  getContainer() const;

        void SetPosSizePixel( Point _rPt,Size _rSize);
        css::uno::Reference< css::uno::XComponentContext > const & getORB() const;

    private:
        void    impl_forceSQLView();
        bool    impl_postViewSwitch( const bool i_bGraphicalDesign, const bool i_bSuccess );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYVIEWSWITCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
