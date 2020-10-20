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
#ifndef INCLUDED_SW_INC_LINEINFO_HXX
#define INCLUDED_SW_INC_LINEINFO_HXX

#include "calbck.hxx"
#include <editeng/numitem.hxx>
#include "swdllapi.h"

class SwCharFormat;
class IDocumentStylePoolAccess;

enum LineNumberPosition
{
    LINENUMBER_POS_LEFT,
    LINENUMBER_POS_RIGHT,
    LINENUMBER_POS_INSIDE,
    LINENUMBER_POS_OUTSIDE
};

class SW_DLLPUBLIC SwLineNumberInfo final : public SwClient /**< purpose of derivation from SwClient:
                                                         character style for displaying the numbers. */
{
    SvxNumberType       m_aType;                  ///< e.g. roman linenumbers
    OUString            m_aDivider;               ///< String for additional interval (vert. lines user defined)
    sal_uInt16              m_nPosFromLeft;       ///< Position for paint
    sal_uInt16              m_nCountBy;           ///< Paint only for every n line
    sal_uInt16              m_nDividerCountBy;    /**< Interval for display of an user defined
                                                       string every n lines */
    LineNumberPosition  m_ePos;                   ///< Where should the display occur (number and divider)
    bool                m_bPaintLineNumbers;      ///< Should anything be displayed?
    bool                m_bCountBlankLines;       ///< Count empty lines?
    bool                m_bCountInFlys;           ///< Count also within FlyFrames?
    bool                m_bRestartEachPage;       /**< Restart counting at the first paragraph of each page
                                                       (even on follows when paragraphs are split) */
    virtual void SwClientNotify( const SwModify&, const SfxHint& ) override;

public:
    SwLineNumberInfo();
    SwLineNumberInfo(const SwLineNumberInfo&);

    SwLineNumberInfo& operator=(const SwLineNumberInfo&);

    SwCharFormat *GetCharFormat( IDocumentStylePoolAccess& rIDSPA ) const;
    void SetCharFormat( SwCharFormat* );

    const SvxNumberType &GetNumType() const             { return m_aType; }
    void                SetNumType( SvxNumberType aNew ){ m_aType = aNew; }

    const OUString& GetDivider() const           { return m_aDivider; }
    void SetDivider( const OUString &r )  { m_aDivider = r; }
    sal_uInt16 GetDividerCountBy() const    { return m_nDividerCountBy; }
    void SetDividerCountBy( sal_uInt16 n )  { m_nDividerCountBy = n; }

    sal_uInt16 GetPosFromLeft() const       { return m_nPosFromLeft; }
    void   SetPosFromLeft( sal_uInt16 n)    { m_nPosFromLeft = n;    }

    sal_uInt16 GetCountBy() const           { return m_nCountBy; }
    void   SetCountBy( sal_uInt16 n)        { m_nCountBy = n;    }

    LineNumberPosition GetPos() const   { return m_ePos; }
    void SetPos( LineNumberPosition eP ){ m_ePos = eP;   }

    bool   IsPaintLineNumbers() const   { return m_bPaintLineNumbers; }
    void   SetPaintLineNumbers( bool b ){ m_bPaintLineNumbers = b;    }

    bool   IsCountBlankLines() const    { return m_bCountBlankLines;  }
    void   SetCountBlankLines( bool b ) { m_bCountBlankLines = b;     }

    bool   IsCountInFlys() const        { return m_bCountInFlys;      }
    void   SetCountInFlys( bool b )     { m_bCountInFlys = b;         }

    bool   IsRestartEachPage() const    { return m_bRestartEachPage;  }
    void   SetRestartEachPage( bool b ) { m_bRestartEachPage = b;     }

    bool   HasCharFormat() const { return GetRegisteredIn() != nullptr; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
