/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include <paralist.hxx>

#include <editeng/editeng.hxx>
#include <editeng/outliner.hxx>
#include <editeng/numdef.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <libxml/xmlwriter.h>


// static
bool ParagraphList::HasChildren( sal_Int32 nPara, const EditEngine& rEditEngine )
{
    sal_Int32 nParaNext = nPara+1;
    if (nParaNext >= rEditEngine.GetParagraphCount())
        return false;
    return rEditEngine.GetNumberingDepth(nParaNext) > rEditEngine.GetNumberingDepth(nPara);
}

// static
bool ParagraphList::HasHiddenChildren( sal_Int32 nPara, const EditEngine& rEditEngine )
{
    sal_Int32 nParaNext = nPara+1;
    if (nParaNext >= rEditEngine.GetParagraphCount())
        return false;
    return ( rEditEngine.GetNumberingDepth(nParaNext) > rEditEngine.GetNumberingDepth(nPara) )
        && !rEditEngine.IsBulletVisible(nParaNext);
}

// static
bool ParagraphList::HasVisibleChildren( sal_Int32 nPara, const EditEngine& rEditEngine )
{
    sal_Int32 nParaNext = nPara+1;
    if (nParaNext >= rEditEngine.GetParagraphCount())
        return false;
    return ( rEditEngine.GetNumberingDepth(nParaNext) > rEditEngine.GetNumberingDepth(nPara) )
        && rEditEngine.IsBulletVisible(nParaNext);
}

// static
sal_Int32 ParagraphList::GetChildCount( sal_Int32 nPara, const EditEngine& rEditEngine )
{
    sal_Int32 nChildCount = 0;
    sal_Int32 nParaNext = nPara+1;
    while ( nParaNext < rEditEngine.GetParagraphCount()
            && ( rEditEngine.GetNumberingDepth(nParaNext) > rEditEngine.GetNumberingDepth(nPara) ) )
    {
        nChildCount++;
        ++nParaNext;
    }
    return nChildCount;
}

// static
sal_Int32 ParagraphList::GetParent( sal_Int32 nPara, const EditEngine& rEditEngine )
{
    sal_Int32 nParaPrev = nPara-1;
    while ( nParaPrev >= 0 && ( rEditEngine.GetNumberingDepth(nParaPrev) >= rEditEngine.GetNumberingDepth(nPara) ) )
    {
        --nParaPrev;
    }

    return nParaPrev;
}

void ParagraphList::Expand( sal_Int32 nParentPos, EditEngine& rEditEngine )
{
    sal_Int32 nChildCount = GetChildCount( nParentPos, rEditEngine );
    sal_Int32 nPos = nParentPos;
    for ( sal_Int32 n = 1; n <= nChildCount; n++  )
    {
        if ( !( rEditEngine.IsBulletVisible(nPos + n) ) )
        {
            rEditEngine.SetBulletVisible(nPos + n, true);
            aVisibleStateChangedHdl.Call( nPos + n );
        }
    }
}

void ParagraphList::Collapse( sal_Int32 nParentPos, EditEngine& rEditEngine )
{
    sal_Int32 nChildCount = GetChildCount( nParentPos, rEditEngine );
    sal_Int32 nPos = nParentPos;

    for ( sal_Int32 n = 1; n <= nChildCount; n++  )
    {
        if ( rEditEngine.IsBulletVisible(nPos+n) )
        {
            rEditEngine.SetBulletVisible(nPos+n, false);
            aVisibleStateChangedHdl.Call( nPos+n );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
