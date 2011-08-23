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

#ifndef _EERDLL2_HXX
#define _EERDLL2_HXX

#include <forbiddencharacterstable.hxx>
#include <vos/ref.hxx>
namespace binfilter {

class SfxPoolItem; 
class SvxAutoCorrect;

class GlobalEditData
{
private:
    SfxPoolItem** 	ppDefItems;
    OutputDevice*	pStdRefDevice;

    Link			aGetAutoCorrectHdl;

    vos::ORef<SvxForbiddenCharactersTable>	xForbiddenCharsTable;

public:
                    GlobalEditData();
                    ~GlobalEditData();

    SfxPoolItem**	GetDefItems();
    OutputDevice*	GetStdRefDevice();

    void			SetGetAutoCorrectHdl( const Link& rHdl ) { aGetAutoCorrectHdl = rHdl; }
    SvxAutoCorrect*	GetAutoCorrect() const { return (SvxAutoCorrect*) aGetAutoCorrectHdl.Call( NULL ); }

    vos::ORef<SvxForbiddenCharactersTable>	GetForbiddenCharsTable();
    void			SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars ) { xForbiddenCharsTable = xForbiddenChars; }
};


}//end of namespace binfilter
#endif //_EERDLL2_HXX

