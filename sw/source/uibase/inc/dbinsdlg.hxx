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
#include <svtools/svmedit.hxx>
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

class SwInsDBColumns : public o3tl::sorted_vector<SwInsDBColumn*, o3tl::less_ptr_to<SwInsDBColumn> >
{
public:
    ~SwInsDBColumns() { DeleteAndDestroyAll(); }
};

class SwInsertDBColAutoPilot : public SfxModalDialog, public utl::ConfigItem
{
    VclPtr<RadioButton>    m_pRbAsTable;
    VclPtr<RadioButton>    m_pRbAsField;
    VclPtr<RadioButton>    m_pRbAsText;

    VclPtr<VclFrame>       m_pHeadFrame;

    VclPtr<ListBox>        m_pLbTableDbColumn;
    VclPtr<ListBox>        m_pLbTextDbColumn;

    VclPtr<VclFrame>       m_pFormatFrame;
    VclPtr<RadioButton>    m_pRbDbFormatFromDb;
    VclPtr<RadioButton>    m_pRbDbFormatFromUsr;
    VclPtr<NumFormatListBox> m_pLbDbFormatFromUsr;

    // Page Text/Field
    VclPtr<PushButton>     m_pIbDbcolToEdit;
    VclPtr<VclMultiLineEdit> m_pEdDbText;
    VclPtr<FixedText>      m_pFtDbParaColl;
    VclPtr<ListBox>        m_pLbDbParaColl;

    // Page Table
    VclPtr<PushButton>     m_pIbDbcolAllTo;
    VclPtr<PushButton>     m_pIbDbcolOneTo;
    VclPtr<PushButton>     m_pIbDbcolOneFrom;
    VclPtr<PushButton>     m_pIbDbcolAllFrom;
    VclPtr<FixedText>      m_pFtTableCol;
    VclPtr<ListBox>        m_pLbTableCol;
    VclPtr<CheckBox>       m_pCbTableHeadon;
    VclPtr<RadioButton>    m_pRbHeadlColnms;
    VclPtr<RadioButton>    m_pRbHeadlEmpty;
    VclPtr<PushButton>     m_pPbTableFormat;
    VclPtr<PushButton>     m_pPbTableAutofmt;

    SwInsDBColumns  aDBColumns;
    const SwDBData  aDBData;

    Link<ListBox&,void>    aOldNumFormatLnk;
    OUString        sNoTmpl;

    SwView*         pView;
    SwTableAutoFormat* pTAutoFormat;

    SfxItemSet*     pTableSet;
    SwTableRep*     pRep;
    sal_Int32       nGBFormatLen;

    DECL_LINK( PageHdl, Button*, void );
    DECL_LINK( AutoFormatHdl, Button*, void );
    DECL_LINK( TableFormatHdl, Button*, void );
    DECL_LINK( DBFormatHdl, Button*, void );
    DECL_LINK( TableToFromHdl, Button*, void );
    DECL_LINK( SelectHdl, ListBox&, void );
    DECL_LINK( DblClickHdl, ListBox&, void );
    DECL_LINK( HeaderHdl, Button*, void );

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
    virtual void dispose() override;

    void DataToDoc( const css::uno::Sequence< css::uno::Any >& rSelection,
        css::uno::Reference< css::sdbc::XDataSource> const & rxSource,
        css::uno::Reference< css::sdbc::XConnection> const & xConnection,
        css::uno::Reference< css::sdbc::XResultSet > const & xResultSet);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
