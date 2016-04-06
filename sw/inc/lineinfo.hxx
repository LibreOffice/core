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

class SW_DLLPUBLIC SwLineNumberInfo : public SwClient /**< purpose of derivation from SwClient:
                                                         character style for displaying the numbers. */
{
    SvxNumberType       aType;                  ///< e.g. roman linenumbers
    OUString            aDivider;               ///< String for additional interval (vert. lines user defined)
    sal_uInt16              nPosFromLeft;       ///< Position for paint
    sal_uInt16              nCountBy;           ///< Paint only for every n line
    sal_uInt16              nDividerCountBy;    /**< Interval for display of an user defined
                                                       string every n lines */
    LineNumberPosition  ePos;                   ///< Where should the display occur (number and divider)
    bool                bPaintLineNumbers;      ///< Should anything be displayed?
    bool                bCountBlankLines;       ///< Count empty lines?
    bool                bCountInFlys;           ///< Count also within FlyFrames?
    bool                bRestartEachPage;       /**< Restart counting at the first paragraph of each page
                                                       (even on follows when paragraphs are splitted) */
protected:
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:
    SwLineNumberInfo();
    SwLineNumberInfo(const SwLineNumberInfo&);

    SwLineNumberInfo& operator=(const SwLineNumberInfo&);

    SwCharFormat *GetCharFormat( IDocumentStylePoolAccess& rIDSPA ) const;
    void SetCharFormat( SwCharFormat* );

    const SvxNumberType &GetNumType() const             { return aType; }
    void                SetNumType( SvxNumberType aNew ){ aType = aNew; }

    const OUString& GetDivider() const           { return aDivider; }
    void SetDivider( const OUString &r )  { aDivider = r; }
    sal_uInt16 GetDividerCountBy() const    { return nDividerCountBy; }
    void SetDividerCountBy( sal_uInt16 n )  { nDividerCountBy = n; }

    sal_uInt16 GetPosFromLeft() const       { return nPosFromLeft; }
    void   SetPosFromLeft( sal_uInt16 n)    { nPosFromLeft = n;    }

    sal_uInt16 GetCountBy() const           { return nCountBy; }
    void   SetCountBy( sal_uInt16 n)        { nCountBy = n;    }

    LineNumberPosition GetPos() const   { return ePos; }
    void SetPos( LineNumberPosition eP ){ ePos = eP;   }

    bool   IsPaintLineNumbers() const   { return bPaintLineNumbers; }
    void   SetPaintLineNumbers( bool b ){ bPaintLineNumbers = b;    }

    bool   IsCountBlankLines() const    { return bCountBlankLines;  }
    void   SetCountBlankLines( bool b ) { bCountBlankLines = b;     }

    bool   IsCountInFlys() const        { return bCountInFlys;      }
    void   SetCountInFlys( bool b )     { bCountInFlys = b;         }

    bool   IsRestartEachPage() const    { return bRestartEachPage;  }
    void   SetRestartEachPage( bool b ) { bRestartEachPage = b;     }

    bool   HasCharFormat() const { return GetRegisteredIn() != nullptr; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
