/*************************************************************************
 *
 *  $RCSfile: UStringDescription_Impl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-24 15:37:08 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef CONNECTIVITY_USTRINGDESCRIPTION_IMPL_HXX
#define CONNECTIVITY_USTRINGDESCRIPTION_IMPL_HXX

#define IMPLEMENT_CONSTASCII_USTRING(name)  \
UStringDescription name(connectivity::dbtools::get##name)

UStringDescription::UStringDescription(PVFN _fCharFkt)
{
    pZeroTerminatedName = (*_fCharFkt)();
    nLength = 0;
    const sal_Char* pIter = pZeroTerminatedName;
    while((*pIter++) != '\0')
        ++nLength;
}
// -------------------------------------------------------------------------
UStringDescription::~UStringDescription()
{
}

IMPLEMENT_CONSTASCII_USTRING(PROPERTY_QUERYTIMEOUT          );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_MAXFIELDSIZE          );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_MAXROWS               );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_CURSORNAME            );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_RESULTSETCONCURRENCY  );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_RESULTSETTYPE         );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_FETCHDIRECTION        );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_FETCHSIZE             );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ESCAPEPROCESSING      );
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_USEBOOKMARKS          );

IMPLEMENT_CONSTASCII_USTRING(PROPERTY_NAME);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_TYPE);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_TYPENAME);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_PRECISION);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_SCALE);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISNULLABLE);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISAUTOINCREMENT);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISROWVERSION);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_DESCRIPTION);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_DEFAULTVALUE);

IMPLEMENT_CONSTASCII_USTRING(PROPERTY_REFERENCEDTABLE);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_UPDATERULE);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_DELETERULE);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_CATALOG);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISUNIQUE);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISPRIMARYKEYINDEX);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISCLUSTERED);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISASCENDING);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_SCHEMANAME);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_CATALOGNAME);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_COMMAND);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_CHECKOPTION);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_PASSWORD);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_REFERENCEDCOLUMN);

IMPLEMENT_CONSTASCII_USTRING(STAT_INVALID_INDEX);

IMPLEMENT_CONSTASCII_USTRING(PROPERTY_FUNCTION);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_TABLENAME);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_REALNAME);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_DBASEPRECISIONCHANGED);
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISCURRENCY);

IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ISBOOKMARKABLE);
//============================================================
//= error messages
//============================================================
IMPLEMENT_CONSTASCII_USTRING(ERRORMSG_SEQUENCE);
IMPLEMENT_CONSTASCII_USTRING(SQLSTATE_SEQUENCE);

#endif // CONNECTIVITY_USTRINGDESCRIPTION_IMPL_HXX

