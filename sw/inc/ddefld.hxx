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
#ifndef INCLUDED_SW_INC_DDEFLD_HXX
#define INCLUDED_SW_INC_DDEFLD_HXX

#include <sfx2/lnkbase.hxx>
#include <svl/hint.hxx>
#include <tools/solar.h>
#include "swdllapi.h"
#include "fldbas.hxx"

class SwDoc;
class SwNode;
class SwNodes;

namespace sw
{
    struct LinkAnchorSearchHint final : public SfxHint
    {
        SwNodes& m_rNodes;
        const SwNode*& m_rpFoundNode;
        LinkAnchorSearchHint(SwNodes& rNodes, const SwNode*& rpFoundNode) : m_rNodes(rNodes), m_rpFoundNode(rpFoundNode) {};
        virtual ~LinkAnchorSearchHint() override;
    };
    struct InRangeSearchHint final : public SfxHint
    {
        const SwNodeOffset m_nSttNd, m_nEndNd;
        bool& m_rIsInRange;
        InRangeSearchHint(const SwNodeOffset nSttNd, const SwNodeOffset nEndNd, bool& rIsInRange)
            : m_nSttNd(nSttNd), m_nEndNd(nEndNd), m_rIsInRange(rIsInRange) {}
    };
}

// FieldType for DDE
class SW_DLLPUBLIC SwDDEFieldType final : public SwFieldType
{
    OUString m_aName;
    OUString m_aExpansion;

    tools::SvRef<sfx2::SvBaseLink> m_RefLink;
    SwDoc* m_pDoc;

    sal_uInt16 m_nRefCount;
    bool m_bCRLFFlag : 1;
    bool m_bDeleted : 1;

    SAL_DLLPRIVATE void RefCntChgd();

public:
    SwDDEFieldType( const OUString& rName, const OUString& rCmd,
                    SfxLinkUpdateMode );
    virtual ~SwDDEFieldType() override;

    const OUString& GetExpansion() const               { return m_aExpansion; }
    void SetExpansion( const OUString& rStr )   { m_aExpansion = rStr;
                                                  m_bCRLFFlag = false; }

    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual OUString GetName() const override;

    virtual void QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual void PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

    OUString const & GetCmd() const;
    void SetCmd( const OUString& aStr );

    SfxLinkUpdateMode GetType() const          { return m_RefLink->GetUpdateMode();  }
    void SetType( SfxLinkUpdateMode nType )    { m_RefLink->SetUpdateMode( nType );  }

    bool IsDeleted() const          { return m_bDeleted; }
    void SetDeleted( bool b )       { m_bDeleted = b; }

    void Disconnect()               { m_RefLink->Disconnect(); }

    const ::sfx2::SvBaseLink& GetBaseLink() const    { return *m_RefLink; }
          ::sfx2::SvBaseLink& GetBaseLink()          { return *m_RefLink; }

    const SwDoc* GetDoc() const     { return m_pDoc; }
          SwDoc* GetDoc()           { return m_pDoc; }
    void SetDoc( SwDoc* pDoc );

    void IncRefCnt() {  if( !m_nRefCount++ && m_pDoc ) RefCntChgd(); }
    void DecRefCnt() {  if( !--m_nRefCount && m_pDoc ) RefCntChgd(); }

    void SetCRLFDelFlag( bool bFlag )    { m_bCRLFFlag = bFlag; }
};

// DDE-field
class SwDDEField final : public SwField
{
private:
    virtual OUString ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwDDEField(SwDDEFieldType*);
    virtual ~SwDDEField() override;

    /** Get parameter via types.
     Name cannot be changed. */
    virtual OUString  GetPar1() const override;

    // Command
    virtual OUString  GetPar2() const override;
    virtual void    SetPar2(const OUString& rStr) override;
};

#endif // INCLUDED_SW_INC_DDEFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
