/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: qualiname.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 16:33:51 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_autodoc.hxx"

#include <precomp.h>
#include <ary/qualiname.hxx>


// NOT FULLY DECLARED SERVICES


namespace ary
{


QualifiedName::QualifiedName( uintt i_nSize )
    :   aNamespace(),
        sLocalName(),
        bIsAbsolute(false),
        bIsFunction()
{
    if (i_nSize > 0)
        aNamespace.reserve(i_nSize);
}

QualifiedName::QualifiedName( const char *        i_sText,
                              const char *        i_sSeparator )
    :   aNamespace(),
        sLocalName(),
        bIsAbsolute(false),
        bIsFunction()
{
    AssignText(i_sText,i_sSeparator);
}

QualifiedName::~QualifiedName()
{
}

void
QualifiedName::AssignText(  const char *        i_sText,
                            const char *        i_sSeparator )
{
    csv_assert(NOT csv::no_str(i_sText) AND NOT csv::no_str(i_sSeparator));
    bIsAbsolute = false;
    bIsFunction = false;
    csv::erase_container( aNamespace );

    uintt nSepLen = strlen(i_sSeparator);
    const char * sNext = i_sText;

    const char * ps = strstr( i_sText, i_sSeparator );
    if (ps == i_sText)
    {
        bIsAbsolute = true;
        sNext = ps + nSepLen;
    }

    for ( ps = strstr(sNext, i_sSeparator);
          ps != 0;
          ps = strstr(sNext, i_sSeparator) )
    {
        String sPart(sNext, ps - sNext);
        aNamespace.push_back(sPart);
        sNext = ps + nSepLen;
    }

    uintt sNameLen = strlen(sNext);
    if ( sNameLen > 2 )
    {
        ps = sNext + sNameLen - 2;
        if (*ps == '(' AND *(ps+1) == ')')
        {
            sNameLen -= 2;
            bIsFunction = true;
        }
    }
    sLocalName = String(sNext,sNameLen);
}


}   // namespace ary
