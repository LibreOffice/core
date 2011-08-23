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

#ifndef _BF_GOODIES_GRAPHICOBJECT_HXX
#define _BF_GOODIES_GRAPHICOBJECT_HXX

#include <svtools/grfmgr.hxx>

// wrapper for GraphicAttr for binfilter
namespace binfilter {
    class BfGraphicAttr : public GraphicAttr
    {
    public:
        BfGraphicAttr();
        ~BfGraphicAttr();
        void			SetTransparency( BYTE cTransparency );
    };
}//end of namespace binfilter

// wrapper for GraphicObject for binfilter
namespace binfilter {
    class BfGraphicObject : public GraphicObject
    {
    protected:
        virtual void GraphicManagerDestroyed();
        virtual	ULONG GetReleaseFromCache() const;
        virtual SvStream* GetSwapStream() const;
        virtual void Load( SvStream& );
        virtual void Save( SvStream& );
        virtual void Assign( const SvDataCopyStream& );

    public:
        TYPEINFO();
        BfGraphicObject( const GraphicManager* pMgr = NULL );
        BfGraphicObject( const Graphic& rGraphic, const GraphicManager* pMgr = NULL );
        BfGraphicObject( const BfGraphicObject& rCacheObj, const GraphicManager* pMgr = NULL );
        BfGraphicObject( const ByteString& rUniqueID, const GraphicManager* pMgr = NULL );
        ~BfGraphicObject();

        BOOL					operator==( const BfGraphicObject& rCacheObj ) const;
        const Graphic&			GetGraphic() const;
        void					SetAttr( const BfGraphicAttr& rAttr );
        const BfGraphicAttr&	GetAttr() const;
        ByteString				GetUniqueID() const;
        BfGraphicObject&		operator=( const BfGraphicObject& rCacheObj );
        void					SetUserData();
        void					SetUserData( const String& rUserData );
        void					SetGraphic( const Graphic& rGraphic );
        void					SetGraphic( const Graphic& rGraphic, const String& rLink );
        void					SetSwapState();
        void					FireSwapInRequest();
        void					FireSwapOutRequest();
        void					StopAnimation( OutputDevice* pOut = NULL, long nExtraData = 0L );
        BOOL					Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                    const BfGraphicAttr* pAttr = NULL, ULONG nFlags = GRFMGR_DRAW_STANDARD );
        String					GetUserData() const;
        void					SetSwapStreamHdl( const Link& rHdl, const ULONG nSwapOutTimeout = 0UL );
        void					ReleaseFromCache();
        String					GetLink() const;
        void					SetLink( const String& rLink );
        BOOL					SwapOut();
        BOOL					SwapOut( SvStream* pOStm );
        BOOL					SwapIn();
        BOOL					SwapIn( SvStream* pIStm );
    };
}//end of namespace binfilter
#endif // _BF_GOODIES_GRAPHICOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
