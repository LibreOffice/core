/*************************************************************************
 *
 *  $RCSfile: qualiname.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:14:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
