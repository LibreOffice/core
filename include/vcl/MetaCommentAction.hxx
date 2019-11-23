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

#ifndef INCLUDED_VCL_METACOMMENTACTION_HXX
#define INCLUDED_VCL_METACOMMENTACTION_HXX

#include <rtl/ustring.hxx>

#include <vcl/MetaAction.hxx>

#include <memory>

class VCL_DLLPUBLIC MetaCommentAction final : public MetaAction
{
private:

    OString             maComment;
    sal_Int32           mnValue;
    sal_uInt32          mnDataSize;
    std::unique_ptr<sal_uInt8[]>
                        mpData;

    SAL_DLLPRIVATE void ImplInitDynamicData( const sal_uInt8* pData, sal_uInt32 nDataSize );

private:
    virtual             ~MetaCommentAction() override;

public:
    explicit            MetaCommentAction();
    explicit            MetaCommentAction( const MetaCommentAction& rAct );
    explicit            MetaCommentAction( const OString& rComment, sal_Int32 nValue = 0, const sal_uInt8* pData = nullptr, sal_uInt32 nDataSize = 0 );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    const OString&      GetComment() const { return maComment; }
    sal_Int32           GetValue() const { return mnValue; }
    sal_uInt32          GetDataSize() const { return mnDataSize; }
    const sal_uInt8*    GetData() const { return mpData.get(); }
};

#endif // INCLUDED_VCL_METACOMMENTACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
