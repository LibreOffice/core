/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "graphicobject.hxx"

namespace binfilter {
    BfGraphicAttr::BfGraphicAttr() : GraphicAttr() {}
    BfGraphicAttr::~BfGraphicAttr() {}
    void			BfGraphicAttr::SetTransparency( BYTE cTransparency ) { GraphicAttr::SetTransparency(cTransparency); }
}//end of namespace binfilter

namespace binfilter {
    TYPEINIT1_AUTOFACTORY( BfGraphicObject, GraphicObject);
    void BfGraphicObject::GraphicManagerDestroyed() { GraphicObject::GraphicManagerDestroyed(); }
    ULONG BfGraphicObject::GetReleaseFromCache() const { return GraphicObject::GetReleaseFromCache(); }
    SvStream* BfGraphicObject::GetSwapStream() const { return GraphicObject::GetSwapStream(); }
    void BfGraphicObject::Load( SvStream& rSt ) { GraphicObject::Load(rSt); }
    void BfGraphicObject::Save( SvStream& rSt ) { GraphicObject::Save(rSt); }
    void BfGraphicObject::Assign( const SvDataCopyStream& rSt ) { GraphicObject::Assign(rSt); }
    BfGraphicObject::BfGraphicObject( const GraphicManager* pMgr ) : GraphicObject(pMgr) {}
    BfGraphicObject::BfGraphicObject( const Graphic& rGraphic, const GraphicManager* pMgr ) : GraphicObject(rGraphic, pMgr) {}
    BfGraphicObject::BfGraphicObject( const BfGraphicObject& rCacheObj, const GraphicManager* pMgr ) : GraphicObject(rCacheObj, pMgr) {}
    BfGraphicObject::BfGraphicObject( const ByteString& rUniqueID, const GraphicManager* pMgr ) : GraphicObject(rUniqueID, pMgr) {}
    BfGraphicObject::~BfGraphicObject() {}

    BOOL					BfGraphicObject::operator==( const BfGraphicObject& rCacheObj ) const { return GraphicObject::operator==(rCacheObj); }
    const Graphic&			BfGraphicObject::GetGraphic() const { return GraphicObject::GetGraphic(); }
    void					BfGraphicObject::SetAttr( const BfGraphicAttr& rAttr ) { GraphicObject::SetAttr(rAttr); }
    const BfGraphicAttr&	BfGraphicObject::GetAttr() const { return (BfGraphicAttr&)GraphicObject::GetAttr(); }
    ByteString				BfGraphicObject::GetUniqueID() const { return GraphicObject::GetUniqueID(); }
    BfGraphicObject&		BfGraphicObject::operator=( const BfGraphicObject& rCacheObj ) { return (BfGraphicObject&)GraphicObject::operator=(rCacheObj); }
    void					BfGraphicObject::SetUserData() { GraphicObject::SetUserData(); }
    void					BfGraphicObject::SetUserData( const String& rUserData ) { GraphicObject::SetUserData(rUserData); }
    void					BfGraphicObject::SetGraphic( const Graphic& rGraphic ) { GraphicObject::SetGraphic(rGraphic); }
    void					BfGraphicObject::SetGraphic( const Graphic& rGraphic, const String& rLink ) { GraphicObject::SetGraphic(rGraphic, rLink); }
    void					BfGraphicObject::SetSwapState() { GraphicObject::SetSwapState(); }
    void					BfGraphicObject::FireSwapInRequest() { GraphicObject::FireSwapInRequest(); }
    void					BfGraphicObject::FireSwapOutRequest() { GraphicObject::FireSwapOutRequest(); }
    void					BfGraphicObject::StopAnimation( OutputDevice* pOut, long nExtraData) { GraphicObject::StopAnimation( pOut, nExtraData); }
    BOOL					BfGraphicObject::Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz, const BfGraphicAttr* pAttr, ULONG nFlags) { return GraphicObject::Draw( pOut, rPt, rSz, pAttr, nFlags); }
    String					BfGraphicObject::GetUserData() const { return GraphicObject::GetUserData(); }
    void					BfGraphicObject::SetSwapStreamHdl( const Link& rHdl, const ULONG nSwapOutTimeout) { GraphicObject::SetSwapStreamHdl( rHdl, nSwapOutTimeout); }
    void					BfGraphicObject::ReleaseFromCache() { GraphicObject::ReleaseFromCache(); }
    String					BfGraphicObject::GetLink() const { return GraphicObject::GetLink(); }
    void					BfGraphicObject::SetLink( const String& rLink ) { GraphicObject::SetLink( rLink ); }
    BOOL					BfGraphicObject::SwapOut() { return GraphicObject::SwapOut(); }
    BOOL					BfGraphicObject::SwapOut( SvStream* pOStm ) { return GraphicObject::SwapOut( pOStm ); }
    BOOL					BfGraphicObject::SwapIn() { return GraphicObject::SwapIn(); }
    BOOL					BfGraphicObject::SwapIn( SvStream* pIStm ) { return GraphicObject::SwapIn( pIStm ); }
}//end of namespace binfilter

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
