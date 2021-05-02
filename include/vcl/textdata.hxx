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

#ifndef INCLUDED_VCL_TEXTDATA_HXX
#define INCLUDED_VCL_TEXTDATA_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <svl/hint.hxx>
#include <vcl/dllapi.h>
#include <memory>

enum class ExtTextInputAttr;

// for Notify, if all paragraphs were deleted
#define TEXT_PARA_ALL               SAL_MAX_UINT32
#define TEXT_INDEX_ALL              SAL_MAX_INT32

class TextPaM
{
private:
    sal_uInt32          mnPara;
    sal_Int32           mnIndex;

public:
                    TextPaM() : mnPara(0), mnIndex(0) {}
                    TextPaM( sal_uInt32 nPara, sal_Int32 nIndex ) : mnPara(nPara), mnIndex(nIndex) {}

    sal_uInt32          GetPara() const     { return mnPara; }
    sal_uInt32&         GetPara()           { return mnPara; }

    sal_Int32           GetIndex() const    { return mnIndex; }
    sal_Int32&          GetIndex()          { return mnIndex; }

    inline bool     operator == ( const TextPaM& rPaM ) const;
    inline bool     operator != ( const TextPaM& rPaM ) const;
    inline bool     operator < ( const TextPaM& rPaM ) const;
    inline bool     operator > ( const TextPaM& rPaM ) const;
};

inline bool TextPaM::operator == ( const TextPaM& rPaM ) const
{
    return ( mnPara == rPaM.mnPara ) && ( mnIndex == rPaM.mnIndex );
}

inline bool TextPaM::operator != ( const TextPaM& rPaM ) const
{
    return !( *this == rPaM );
}

inline bool TextPaM::operator < ( const TextPaM& rPaM ) const
{
    return ( mnPara < rPaM.mnPara ) ||
           ( ( mnPara == rPaM.mnPara ) && mnIndex < rPaM.mnIndex );
}

inline bool TextPaM::operator > ( const TextPaM& rPaM ) const
{
    return ( mnPara > rPaM.mnPara ) ||
           ( ( mnPara == rPaM.mnPara ) && mnIndex > rPaM.mnIndex );
}

class VCL_DLLPUBLIC TextSelection
{
private:
    TextPaM         maStartPaM;
    TextPaM         maEndPaM;

public:
                    TextSelection();
                    TextSelection( const TextPaM& rPaM );
                    TextSelection( const TextPaM& rStart, const TextPaM& rEnd );

    const TextPaM&  GetStart() const    { return maStartPaM; }
    TextPaM&        GetStart()          { return maStartPaM; }

    const TextPaM&  GetEnd() const      { return maEndPaM; }
    TextPaM&        GetEnd()            { return maEndPaM; }

    void            Justify();

    bool            HasRange() const    { return maStartPaM != maEndPaM; }

    inline bool     operator == ( const TextSelection& rSel ) const;
    inline bool     operator != ( const TextSelection& rSel ) const;
};

inline bool TextSelection::operator == ( const TextSelection& rSel ) const
{
    return ( ( maStartPaM == rSel.maStartPaM ) && ( maEndPaM == rSel.maEndPaM ) );
}

inline bool TextSelection::operator != ( const TextSelection& rSel ) const
{
    return !( *this == rSel );
}

class VCL_DLLPUBLIC TextHint : public SfxHint
{
private:
    sal_Int32   mnValue;

public:
    TextHint( SfxHintId nId );
    TextHint( SfxHintId nId, sal_Int32 nValue );

    sal_Int32   GetValue() const        { return mnValue; }
};

struct TEIMEInfos
{
    OUString    aOldTextAfterStartPos;
    std::unique_ptr<ExtTextInputAttr[]> pAttribs;
    TextPaM     aPos;
    sal_Int32   nLen;
    bool        bWasCursorOverwrite;

    TEIMEInfos(const TextPaM& rPos, const OUString& rOldTextAfterStartPos);
    ~TEIMEInfos();

    void CopyAttribs(const ExtTextInputAttr* pA, sal_Int32 nL);
    void DestroyAttribs();
};

#endif // INCLUDED_VCL_TEXTDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
