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
#ifndef SW_LINEINFO_HXX
#define SW_LINEINFO_HXX

#include "calbck.hxx"
#include <editeng/numitem.hxx>
#include "swdllapi.h"

class SwCharFmt;
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
    OUString            aDivider;               ///< String for aditional interval (vert. lines user defined)
    sal_uInt16              nPosFromLeft;       ///< Position for paint
    sal_uInt16              nCountBy;           ///< Paint only for every n line
    sal_uInt16              nDividerCountBy;    /**< Interval for display of an user defined
                                                   string every n lines */
    LineNumberPosition  ePos;                   ///< Where should the display occur (number and divicer)
    sal_Bool                bPaintLineNumbers;  ///< Should anything be displayed?
    sal_Bool                bCountBlankLines;   ///< Count empty lines?
    sal_Bool                bCountInFlys;       ///< Count also within FlyFrames?
    sal_Bool                bRestartEachPage;   /**< Restart counting at the first paragraph of each page
                                                    (even on follows when paragraphs are splitted) */
protected:
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    SwLineNumberInfo();
    SwLineNumberInfo(const SwLineNumberInfo&);

    SwLineNumberInfo& operator=(const SwLineNumberInfo&);
    bool operator==( const SwLineNumberInfo& rInf ) const;

    SwCharFmt *GetCharFmt( IDocumentStylePoolAccess& rIDSPA ) const;
    void SetCharFmt( SwCharFmt* );

    const SvxNumberType &GetNumType() const             { return aType; }
    void                SetNumType( SvxNumberType aNew ){ aType = aNew; }

    OUString GetDivider() const           { return aDivider; }
    void SetDivider( const OUString &r )  { aDivider = r; }
    sal_uInt16 GetDividerCountBy() const    { return nDividerCountBy; }
    void SetDividerCountBy( sal_uInt16 n )  { nDividerCountBy = n; }

    sal_uInt16 GetPosFromLeft() const       { return nPosFromLeft; }
    void   SetPosFromLeft( sal_uInt16 n)    { nPosFromLeft = n;    }

    sal_uInt16 GetCountBy() const           { return nCountBy; }
    void   SetCountBy( sal_uInt16 n)        { nCountBy = n;    }

    LineNumberPosition GetPos() const   { return ePos; }
    void SetPos( LineNumberPosition eP ){ ePos = eP;   }

    sal_Bool   IsPaintLineNumbers() const   { return bPaintLineNumbers; }
    void   SetPaintLineNumbers( sal_Bool b ){ bPaintLineNumbers = b;    }

    sal_Bool   IsCountBlankLines() const    { return bCountBlankLines;  }
    void   SetCountBlankLines( sal_Bool b ) { bCountBlankLines = b;     }

    sal_Bool   IsCountInFlys() const        { return bCountInFlys;      }
    void   SetCountInFlys( sal_Bool b )     { bCountInFlys = b;         }

    sal_Bool   IsRestartEachPage() const    { return bRestartEachPage;  }
    void   SetRestartEachPage( sal_Bool b ) { bRestartEachPage = b;     }

    bool   HasCharFormat() const { return GetRegisteredIn() != 0; }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
