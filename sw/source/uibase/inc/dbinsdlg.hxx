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

#include <vcl/weld.hxx>
#include <sfx2/basedlgs.hxx>
#include <unotools/configitem.hxx>
#include "numfmtlb.hxx"
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

class SwInsDBColumns : public o3tl::sorted_vector<std::unique_ptr<SwInsDBColumn>, o3tl::less_uniqueptr_to<SwInsDBColumn> >
{
};

class SwInsertDBColAutoPilot : public SfxDialogController, public utl::ConfigItem
{
    SwInsDBColumns  aDBColumns;
    const SwDBData  aDBData;

    OUString const  sNoTmpl;

    SwView*         pView;
    std::unique_ptr<SwTableAutoFormat> m_xTAutoFormat;

    std::unique_ptr<SfxItemSet>  pTableSet;
    std::unique_ptr<SwTableRep>  pRep;
    sal_Int32       nGBFormatLen;

    std::unique_ptr<weld::RadioButton> m_xRbAsTable;
    std::unique_ptr<weld::RadioButton> m_xRbAsField;
    std::unique_ptr<weld::RadioButton> m_xRbAsText;

    std::unique_ptr<weld::Frame> m_xHeadFrame;

    std::unique_ptr<weld::TreeView> m_xLbTableDbColumn;
    std::unique_ptr<weld::TreeView> m_xLbTextDbColumn;

    std::unique_ptr<weld::Frame> m_xFormatFrame;
    std::unique_ptr<weld::RadioButton> m_xRbDbFormatFromDb;
    std::unique_ptr<weld::RadioButton> m_xRbDbFormatFromUsr;
    std::unique_ptr<SwNumFormatListBox> m_xLbDbFormatFromUsr;

    // Page Text/Field
    std::unique_ptr<weld::Button> m_xIbDbcolToEdit;
    std::unique_ptr<weld::TextView> m_xEdDbText;
    std::unique_ptr<weld::Label> m_xFtDbParaColl;
    std::unique_ptr<weld::ComboBox> m_xLbDbParaColl;

    // Page Table
    std::unique_ptr<weld::Button> m_xIbDbcolAllTo;
    std::unique_ptr<weld::Button> m_xIbDbcolOneTo;
    std::unique_ptr<weld::Button> m_xIbDbcolOneFrom;
    std::unique_ptr<weld::Button> m_xIbDbcolAllFrom;
    std::unique_ptr<weld::Label>  m_xFtTableCol;
    std::unique_ptr<weld::TreeView> m_xLbTableCol;
    std::unique_ptr<weld::CheckButton> m_xCbTableHeadon;
    std::unique_ptr<weld::RadioButton> m_xRbHeadlColnms;
    std::unique_ptr<weld::RadioButton> m_xRbHeadlEmpty;
    std::unique_ptr<weld::Button> m_xPbTableFormat;
    std::unique_ptr<weld::Button> m_xPbTableAutofmt;

    DECL_LINK( PageHdl, weld::Button&, void );
    DECL_LINK( AutoFormatHdl, weld::Button&, void );
    DECL_LINK( TableFormatHdl, weld::Button&, void );
    DECL_LINK( DBFormatHdl, weld::Button&, void );
    DECL_LINK( TableToFromHdl, weld::Button&, void );
    DECL_LINK( TVSelectHdl, weld::TreeView&, void );
    DECL_LINK( CBSelectHdl, weld::ComboBox&, void );
    DECL_LINK( DblClickHdl, weld::TreeView&, void );
    DECL_LINK( HeaderHdl, weld::Button&, void );

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

    virtual ~SwInsertDBColAutoPilot() override;

    void DataToDoc( const css::uno::Sequence< css::uno::Any >& rSelection,
        css::uno::Reference< css::sdbc::XDataSource> const & rxSource,
        css::uno::Reference< css::sdbc::XConnection> const & xConnection,
        css::uno::Reference< css::sdbc::XResultSet > const & xResultSet);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
