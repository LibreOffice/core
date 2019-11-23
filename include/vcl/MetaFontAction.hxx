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

#ifndef INCLUDED_VCL_METAFONTACTION_HXX
#define INCLUDED_VCL_METAFONTACTION_HXX

#include <vcl/font.hxx>
#include <vcl/MetaAction.hxx>

class OutputDevice;
class SvStream;

class VCL_DLLPUBLIC MetaFontAction final : public MetaAction
{
private:

    vcl::Font           maFont;

public:
                        MetaFontAction();
    MetaFontAction(MetaFontAction const &) = default;
    MetaFontAction(MetaFontAction &&) = default;
    MetaFontAction & operator =(MetaFontAction const &) = delete; // due to MetaAction
    MetaFontAction & operator =(MetaFontAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaFontAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaFontAction( const vcl::Font& );

    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const vcl::Font&    GetFont() const { return maFont; }
};

#endif // INCLUDED_VCL_METAFONTACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
