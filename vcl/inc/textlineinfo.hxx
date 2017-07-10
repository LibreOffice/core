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

#ifndef INCLUDED_VCL_INC_TEXTLINEINFO_HXX
#define INCLUDED_VCL_INC_TEXTLINEINFO_HXX

#include <memory>
#include <vector>

class ImplTextLineInfo
{
private:
    long        mnWidth;
    sal_Int32   mnIndex;
    sal_Int32   mnLen;

public:
   ImplTextLineInfo( long nWidth, sal_Int32 nIndex, sal_Int32 nLen )
   {
       mnWidth = nWidth;
       mnIndex = nIndex;
       mnLen   = nLen;
   }

    long       GetWidth() const { return mnWidth; }
    sal_Int32  GetIndex() const { return mnIndex; }
    sal_Int32  GetLen() const { return mnLen; }
};

#define MULTITEXTLINEINFO_RESIZE    16

class ImplMultiTextLineInfo
{
public:
                ImplMultiTextLineInfo();
                ~ImplMultiTextLineInfo();

    void        AddLine( ImplTextLineInfo* pLine );
    void        Clear();

    ImplTextLineInfo* GetLine( sal_Int32 nLine ) const
                            { return mvLines[nLine].get(); }
    sal_Int32   Count() const { return mvLines.size(); }

private:
    ImplMultiTextLineInfo( const ImplMultiTextLineInfo& ) = delete;
    ImplMultiTextLineInfo& operator=( const ImplMultiTextLineInfo& ) = delete;

    std::vector<std::unique_ptr<ImplTextLineInfo>>  mvLines;

};

#endif // INCLUDED_VCL_INC_TEXTLINEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
