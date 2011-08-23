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
#ifndef _SWBLOCKS_HXX
#define _SWBLOCKS_HXX

#include <bf_so3/persist.hxx>
/*N*/#include <tools/debug.hxx> //addded for stripping 
class SvxMacroTableDtor; 
namespace binfilter {

class SfxMedium;
class SwPaM;
class SwDoc;

class SvXMLTextBlocks;
// Name eines Textblocks:




class SwImpBlocks
{
    friend class SwTextBlocks;
protected:
    String aFile;						// physikalischer Dateiname
    String aName;						// logischer Name der Datei
    SwImpBlocks( const String&, BOOL = FALSE ){DBG_ASSERT(0, "STRIP")}; //STRIP001 ;
public:
    const String& GetFileName() const {return aFile;} 	// phys. Dateinamen liefern
};




class Sw3Io;
class Sw3IoImp;

class Sw3TextBlocks : public SwImpBlocks
{

public:
    Sw3TextBlocks( const String& rFile): SwImpBlocks( rFile ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	Sw3TextBlocks( const String& );



    // Methoden fuer die neue Autokorrektur



};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
