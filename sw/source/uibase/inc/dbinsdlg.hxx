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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DBINSDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DBINSDLG_HXX

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/weld.hxx>
#include <svtools/svmedit.hxx>
#include <sfx2/basedlgs.hxx>
#include <unotools/configitem.hxx>
#include <numfmtlb.hxx>
#include <swdbdata.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <o3tl/sorted_vector.hxx>

#include <memory>
#include <vector>

namespace com{namespace sun{namespace star{
    namespace sdbcx{
    class XColumnsSupplier;
    }
    namespace sdbc{
    class XDataSource;
    class XConnection;
    class XResultSet;
    }
}}}

class SwTableAutoFormat;
class SwView;
class SfxItemSet;
class SwTableRep;
struct DB_Column;

typedef std::vector<std::unique_ptr<DB_Column>> DB_Columns;

struct SwInsDBColumn
{
    OUString sColumn, sUsrNumFormat;
    sal_Int32 nDBNumFormat;
    sal_uInt32 nUsrNumFormat;
    LanguageType eUsrNumFormatLng;
    bool bHasFormat : 1;
    bool bIsDBFormat : 1;

    SwInsDBColumn( const OUString& rStr )
        : sColumn( rStr ),
        nDBNumFormat( 0 ),
        nUsrNumFormat( 0 ),
        eUsrNumFormatLng( LANGUAGE_SYSTEM ),
        bHasFormat(false),
        bIsDBFormat(true)
    {}

    bool operator<( const SwInsDBColumn& rCmp ) const;
};

class SwInsDBColumns : public o3tl::sorted_vector<SwInsDBColumn*, o3tl::less_ptr_to<SwInsDBColumn> >
{
public:
    ~SwInsDBColumns() { DeleteAndDestroyAll(); }
};

class SwInsertDBColAutoPilot : public utl::ConfigItem
{
    std::unique_ptr<Weld::Builder> m_xBuilder;
    std::unique_ptr<Weld::Dialog> m_xDialog;
    std::unique_ptr<Weld::RadioButton> m_xRbAsTable;
    std::unique_ptr<Weld::RadioButton> m_xRbAsField;
    std::unique_ptr<Weld::RadioButton> m_xRbAsText;
    std::unique_ptr<Weld::Frame> m_xHeadFrame;

    std::unique_ptr<Weld::TreeView> m_xLbTableDbColumn;
    std::unique_ptr<Weld::Widget> m_xLbTableDbColumnScroll;
    std::unique_ptr<Weld::TreeView> m_xLbTextDbColumn;
    std::unique_ptr<Weld::Widget> m_xLbTextDbColumnScroll;

    std::unique_ptr<Weld::Frame> m_xFormatFrame;
    std::unique_ptr<Weld::RadioButton> m_xRbDbFormatFromDb;
    std::unique_ptr<Weld::RadioButton> m_xRbDbFormatFromUsr;
    std::unique_ptr<NumFormatComboBoxText> m_xLbDbFormatFromUsr;

    // Page Text/Field
    std::unique_ptr<Weld::Button> m_xIbDbcolToEdit;
    std::unique_ptr<Weld::TextView> m_xEdDbText;
    std::unique_ptr<Weld::Widget> m_xEdDbTextScroll;
    std::unique_ptr<Weld::Label> m_xFtDbParaColl;
    std::unique_ptr<Weld::ComboBoxText> m_xLbDbParaColl;

    // Page Table
    std::unique_ptr<Weld::Button> m_xIbDbcolAllTo;
    std::unique_ptr<Weld::Button> m_xIbDbcolOneTo;
    std::unique_ptr<Weld::Button> m_xIbDbcolOneFrom;
    std::unique_ptr<Weld::Button> m_xIbDbcolAllFrom;
    std::unique_ptr<Weld::Label> m_xFtTableCol;
    bool m_bLbTableColLastActive;
    std::unique_ptr<Weld::TreeView> m_xLbTableCol;
    std::unique_ptr<Weld::Widget> m_xLbTableColScroll;
    std::unique_ptr<Weld::CheckButton> m_xCbTableHeadon;
    std::unique_ptr<Weld::RadioButton> m_xRbHeadlColnms;
    std::unique_ptr<Weld::RadioButton> m_xRbHeadlEmpty;
    std::unique_ptr<Weld::Button> m_xPbTableFormat;
    std::unique_ptr<Weld::Button> m_xPbTableAutofmt;

    SwInsDBColumns  aDBColumns;
    const SwDBData  aDBData;

    Link<NumFormatComboBoxText&, void> aOldNumFormatLnk;
    OUString        sNoTmpl;

    SwView*         pView;
    std::unique_ptr<SwTableAutoFormat> m_xTAutoFormat;

    SfxItemSet*     pTableSet;
    SwTableRep*     pRep;
    sal_Int32       nGBFormatLen;

    DECL_LINK(PageHdl, Weld::ToggleButton&, void);
    DECL_LINK(AutoFormatHdl, Weld::Button&, void);
    DECL_LINK(TableFormatHdl, Weld::Button&, void);
    DECL_LINK(DBFormatHdl, Weld::ToggleButton&, void);
    DECL_LINK(TableToFromHdl, Weld::Button&, void);
    DECL_LINK(SelectHdl, Weld::TreeView&, void);
    DECL_LINK(ChangeHdl, NumFormatComboBoxText&, void);
    void DoSelect(const OUString& rString, void* pWidget);
    DECL_LINK(DblClickHdl, Weld::TreeView&, void);
    DECL_LINK(HeaderHdl, Weld::ToggleButton&, void);

    bool SplitTextToColArr( const OUString& rText, DB_Columns& rColArr, bool bInsField );
    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void            ImplCommit() override;
    void                    Load();

    // set the tables - properties
    void SetTabSet();

public:
    SwInsertDBColAutoPilot( SwView& rView,
        css::uno::Reference< css::sdbc::XDataSource> const & rxSource,
        css::uno::Reference<css::sdbcx::XColumnsSupplier> const & xColSupp,
        const SwDBData& rData  );

    short Execute()
    {
        short nRet = m_xDialog->run();
        m_xDialog->hide();
        return nRet;
    }

    ~SwInsertDBColAutoPilot();

    void DataToDoc( const css::uno::Sequence< css::uno::Any >& rSelection,
        css::uno::Reference< css::sdbc::XDataSource> const & rxSource,
        css::uno::Reference< css::sdbc::XConnection> const & xConnection,
        css::uno::Reference< css::sdbc::XResultSet > const & xResultSet);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
