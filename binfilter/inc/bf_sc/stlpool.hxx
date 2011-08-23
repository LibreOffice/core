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

#ifndef SC_STLPOOL_HXX
#define SC_STLPOOL_HXX

#include <bf_svtools/style.hxx>
namespace binfilter {


class ScStyleSheet;
class ScDocument;

class ScStyleSheetPool : public SfxStyleSheetPool
{
public:
                        ScStyleSheetPool( SfxItemPool&	rPool,
                                          ScDocument*	pDocument );
    virtual 			~ScStyleSheetPool();

    void				SetDocument( ScDocument* pDocument );
    ScDocument*			GetDocument() const { return pDoc; }


    void				SetActualStyleSheet ( SfxStyleSheetBase* pActStyleSheet )
                                { pActualStyleSheet = pActStyleSheet; }

    SfxStyleSheetBase*	GetActualStyleSheet ()
                                { return pActualStyleSheet; }

    void				CreateStandardStyles();
    void				UpdateStdNames();



    void				SetForceStdName( const String* pSet );
    const String*		GetForceStdName() const	{ return pForceStdName; }

    virtual SfxStyleSheetBase& Make( const String&, SfxStyleFamily eFam,
                                     USHORT nMask = 0xffff, USHORT nPos = 0xffff );

    void                ConvertFontsAfterLoad();     // old binary file format

protected:
    virtual SfxStyleSheetBase* Create( const String&	rName,
                                       SfxStyleFamily	eFamily,
                                       USHORT			nMask);
    virtual SfxStyleSheetBase* Create( const SfxStyleSheetBase& rStyle );

private:
    SfxStyleSheetBase*	pActualStyleSheet;
    ScDocument*			pDoc;
    const String*		pForceStdName;
};

} //namespace binfilter
#endif	   // SC_STLPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
