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

#include <editeng/legacyitem.hxx>
#include <unotools/fontdefs.hxx>
#include <tools/tenccvt.hxx>
#include <tools/stream.hxx>
#include <comphelper/fileformat.h>
#include <vcl/graph.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/TypeSerializer.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editerr.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/shaditem.hxx>
#include <tools/GenericTypeSerializer.hxx>


namespace legacy
{
    namespace SvxBrush
    {
        const sal_uInt16 LOAD_GRAPHIC = (sal_uInt16(0x0001));
        const sal_uInt16 LOAD_LINK = (sal_uInt16(0x0002));
        const sal_uInt16 LOAD_FILTER = (sal_uInt16(0x0004));

        void Create(SvxBrushItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            bool bTrans;
            Color aTempColor;
            Color aTempFillColor;
            sal_Int8 nStyle;

            rStrm.ReadCharAsBool( bTrans );
            TypeSerializer aSerializer(rStrm);
            aSerializer.readColor(aTempColor);
            aSerializer.readColor(aTempFillColor);
            rStrm.ReadSChar( nStyle );

            switch ( nStyle )
            {
                case 8: // BRUSH_25:
                {
                    sal_uInt32  nRed    = aTempColor.GetRed();
                    sal_uInt32  nGreen  = aTempColor.GetGreen();
                    sal_uInt32  nBlue   = aTempColor.GetBlue();
                    nRed   += static_cast<sal_uInt32>(aTempFillColor.GetRed())*2;
                    nGreen += static_cast<sal_uInt32>(aTempFillColor.GetGreen())*2;
                    nBlue  += static_cast<sal_uInt32>(aTempFillColor.GetBlue())*2;
                    rItem.SetColor(Color( static_cast<sal_Int8>(nRed/3), static_cast<sal_Int8>(nGreen/3), static_cast<sal_Int8>(nBlue/3) ));
                }
                break;

                case 9: // BRUSH_50:
                {
                    sal_uInt32  nRed    = aTempColor.GetRed();
                    sal_uInt32  nGreen  = aTempColor.GetGreen();
                    sal_uInt32  nBlue   = aTempColor.GetBlue();
                    nRed   += static_cast<sal_uInt32>(aTempFillColor.GetRed());
                    nGreen += static_cast<sal_uInt32>(aTempFillColor.GetGreen());
                    nBlue  += static_cast<sal_uInt32>(aTempFillColor.GetBlue());
                    rItem.SetColor(Color( static_cast<sal_Int8>(nRed/2), static_cast<sal_Int8>(nGreen/2), static_cast<sal_Int8>(nBlue/2) ));
                }
                break;

                case 10: // BRUSH_75:
                {
                    sal_uInt32  nRed    = aTempColor.GetRed()*2;
                    sal_uInt32  nGreen  = aTempColor.GetGreen()*2;
                    sal_uInt32  nBlue   = aTempColor.GetBlue()*2;
                    nRed   += static_cast<sal_uInt32>(aTempFillColor.GetRed());
                    nGreen += static_cast<sal_uInt32>(aTempFillColor.GetGreen());
                    nBlue  += static_cast<sal_uInt32>(aTempFillColor.GetBlue());
                    rItem.SetColor(Color( static_cast<sal_Int8>(nRed/3), static_cast<sal_Int8>(nGreen/3), static_cast<sal_Int8>(nBlue/3) ));
                }
                break;

                case 0: // BRUSH_NULL:
                    rItem.SetColor(COL_TRANSPARENT);
                break;

                default:
                    rItem.SetColor(aTempColor);
            }

            if ( nItemVersion < BRUSH_GRAPHIC_VERSION )
                return;

            sal_uInt16 nDoLoad = 0;
            sal_Int8 nPos;

            rStrm.ReadUInt16( nDoLoad );

            if ( nDoLoad & LOAD_GRAPHIC )
            {
                Graphic aGraphic;
                aSerializer.readGraphic(aGraphic);
                rItem.SetGraphicObject(GraphicObject(std::move(aGraphic)));

                if( SVSTREAM_FILEFORMAT_ERROR == rStrm.GetError() )
                {
                    rStrm.ResetError();
                    rStrm.SetError( ERRCODE_SVX_GRAPHIC_WRONG_FILEFORMAT.MakeWarning() );
                }
            }

            if ( nDoLoad & LOAD_LINK )
            {
                // UNICODE: rStrm >> aRel;
                OUString aRel = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

                // TODO/MBA: how can we get a BaseURL here?!
                OSL_FAIL("No BaseURL!");
                OUString aAbs = INetURLObject::GetAbsURL( u"", aRel );
                DBG_ASSERT( !aAbs.isEmpty(), "Invalid URL!" );
                rItem.SetGraphicLink(aAbs);
            }

            if ( nDoLoad & LOAD_FILTER )
            {
                // UNICODE: rStrm >> maStrFilter;
                rItem.SetGraphicFilter(rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet()));
            }

            rStrm.ReadSChar( nPos );

            rItem.SetGraphicPos(static_cast<SvxGraphicPosition>(nPos));
        }

        SvStream& Store(const SvxBrushItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteBool( false );
            TypeSerializer aSerializer(rStrm);
            aSerializer.writeColor(rItem.GetColor());
            aSerializer.writeColor(rItem.GetColor());
            rStrm.WriteSChar( rItem.GetColor().IsTransparent() ? 0 : 1 ); //BRUSH_NULL : BRUSH_SOLID

            sal_uInt16 nDoLoad = 0;
            const GraphicObject* pGraphicObject(rItem.GetGraphicObject());

            if (nullptr != pGraphicObject && rItem.GetGraphicLink().isEmpty())
                nDoLoad |= LOAD_GRAPHIC;
            if ( !rItem.GetGraphicLink().isEmpty() )
                nDoLoad |= LOAD_LINK;
            if ( !rItem.GetGraphicFilter().isEmpty() )
                nDoLoad |= LOAD_FILTER;
            rStrm.WriteUInt16( nDoLoad );

            if (nullptr != pGraphicObject && rItem.GetGraphicLink().isEmpty())
            {
                aSerializer.writeGraphic(pGraphicObject->GetGraphic());
            }
            if ( !rItem.GetGraphicLink().isEmpty() )
            {
                OSL_FAIL("No BaseURL!");
                // TODO/MBA: how to get a BaseURL?!
                OUString aRel = INetURLObject::GetRelURL( u"", rItem.GetGraphicLink() );
                // UNICODE: rStrm << aRel;
                rStrm.WriteUniOrByteString(aRel, rStrm.GetStreamCharSet());
            }
            if ( !rItem.GetGraphicFilter().isEmpty() )
            {
                // UNICODE: rStrm << rItem.GetGraphicFilter();
                rStrm.WriteUniOrByteString(rItem.GetGraphicFilter(), rStrm.GetStreamCharSet());
            }
            rStrm.WriteSChar( rItem.GetGraphicPos() );
            return rStrm;
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
