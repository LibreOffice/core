/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eerdll2.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2007-06-19 15:58:17 $
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

#ifndef _EERDLL2_HXX
#define _EERDLL2_HXX

#ifndef _COM_SUN_STAR_LINGUISTIC2_XLANGUAGEGUESSING_HPP_
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#endif

#include <forbiddencharacterstable.hxx>
#include <vos/ref.hxx>

class SfxPoolItem;

class GlobalEditData
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLanguageGuessing >  xLanguageGuesser;
    SfxPoolItem**   ppDefItems;
    OutputDevice*   pStdRefDevice;

    vos::ORef<SvxForbiddenCharactersTable>  xForbiddenCharsTable;

public:
                    GlobalEditData();
                    ~GlobalEditData();

    SfxPoolItem**   GetDefItems();
    OutputDevice*   GetStdRefDevice();

    vos::ORef<SvxForbiddenCharactersTable>  GetForbiddenCharsTable();
    void            SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars ) { xForbiddenCharsTable = xForbiddenChars; }
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLanguageGuessing > GetLanguageGuesser();
};


#endif //_EERDLL2_HXX

