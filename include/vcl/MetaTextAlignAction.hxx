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

#ifndef INCLUDED_VCL_METATEXTALIGNACTION_HXX
#define INCLUDED_VCL_METATEXTALIGNACTION_HXX

#include <tools/fontenum.hxx>

#include <vcl/MetaAction.hxx>

class OutputDevice;
class SvStream;

class VCL_DLLPUBLIC MetaTextAlignAction final : public MetaAction
{
private:

    TextAlign           maAlign;

public:
                        MetaTextAlignAction();
    MetaTextAlignAction(MetaTextAlignAction const &) = default;
    MetaTextAlignAction(MetaTextAlignAction &&) = default;
    MetaTextAlignAction & operator =(MetaTextAlignAction const &) = delete; // due to MetaAction
    MetaTextAlignAction & operator =(MetaTextAlignAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextAlignAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaTextAlignAction( TextAlign eAlign );

    TextAlign           GetTextAlign() const { return maAlign; }
};

#endif // INCLUDED_VCL_METATEXTALIGNACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
