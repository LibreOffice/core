/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: forbiddencharacterstable.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:44:41 $
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

#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#define _FORBIDDENCHARACTERSTABLE_HXX

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

#include <vos/refernce.hxx>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory;
}}}}

struct ForbiddenCharactersInfo
{
    com::sun::star::i18n::ForbiddenCharacters aForbiddenChars;
    BOOL bTemporary;
};

DECLARE_TABLE( SvxForbiddenCharactersTableImpl, ForbiddenCharactersInfo* )

class SVX_DLLPUBLIC SvxForbiddenCharactersTable : public SvxForbiddenCharactersTableImpl, public vos::OReference
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;

public:
            SvxForbiddenCharactersTable( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF, USHORT nISize = 4, USHORT nGrow = 4 );
            ~SvxForbiddenCharactersTable();

    const com::sun::star::i18n::ForbiddenCharacters* GetForbiddenCharacters( USHORT nLanuage, BOOL bGetDefault ) const;
    void    SetForbiddenCharacters(  USHORT nLanuage , const com::sun::star::i18n::ForbiddenCharacters& );
    void    ClearForbiddenCharacters( USHORT nLanuage );
};

#endif // _FORBIDDENCHARACTERSTABLE_HXX

