/*************************************************************************
 *
 *  $RCSfile: filewriter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-15 19:09:54 $
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
#include <stdio.h>
#include <string.h>
#include "LocaleNode.hxx"
//-----------------------------------------
// The document handler, which is needed for the saxparser
// The Documenthandler for reading sax
//-----------------------------------------
OFileWriter::OFileWriter(const char *pcFile ) {

    strncpy( m_pcFile , pcFile, 1024 );
    printf("file generated=%s\n", m_pcFile);
    m_f = fopen( m_pcFile , "w" );
}

OFileWriter::~OFileWriter() {
    if(m_f)
        fclose( m_f );
}

void OFileWriter::writeInt(sal_Int16 nb) const
{

    fprintf(m_f, "%d", nb);

}

void OFileWriter::writeAsciiString(const char* str) const
{

    fprintf(m_f, "%s", str);

}


void OFileWriter::writeStringCharacters(const ::rtl::OUString& str) const
{
    for(int i = 0; i < str.getLength(); i++)
        fprintf(m_f, "0x%x, ", str[i]);

}


void OFileWriter::flush(void) const
{
    fflush( m_f );
}

void OFileWriter::closeOutput(void) const
{
    if(m_f)
    {
        fclose( m_f );
        const_cast< OFileWriter * > ( this )->m_f = 0;
    }
}

