/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: initui.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:42:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _INITUI_HXX
#define _INITUI_HXX

#ifndef _SOLAR_H
#include "tools/solar.h"
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

/*
 * Forward Declarations
 */
class String;
class SwThesaurus;
class SvStringsDtor;

/*
 * Extern Definitions
 */
extern  SwThesaurus*    pThes;

extern  String*         pOldGrfCat;
extern  String*         pOldTabCat;
extern  String*         pOldFrmCat;

extern  String*         pCurrGlosGroup;

//CHINA001  add for swui to access global variables in sw. Begin
SW_DLLPUBLIC String* GetOldGrfCat();
SW_DLLPUBLIC String* GetOldTabCat();
SW_DLLPUBLIC String* GetOldFrmCat();
SW_DLLPUBLIC String* GetOldDrwCat();

SW_DLLPUBLIC String* GetCurrGlosGroup();
SW_DLLPUBLIC void SetCurrGlosGroup(String* pStr);
//CHINA001 End for add

extern SvStringsDtor*   pDBNameList;

extern SvStringsDtor*   pAuthFieldNameList;
extern SvStringsDtor*   pAuthFieldTypeList;

// stellt die Textbausteinverwaltung zur Verfuegung
class SwGlossaries;
SW_DLLPUBLIC SwGlossaries* GetGlossaries();

class SwGlossaryList;

BOOL HasGlossaryList();
SwGlossaryList* GetGlossaryList();

extern  void _InitUI();
extern  void _FinitUI();
extern  void _InitSpell();
extern  void _FinitSpell();


#endif
