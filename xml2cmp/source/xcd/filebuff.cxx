/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filebuff.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:56:54 $
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

#include "filebuff.hxx"

#include <string.h>
#include <fstream>
#include <ctype.h>


bool
LoadXmlFile( Buffer &           o_rBuffer,
             const char *       i_sXmlFilePath )
{
    std::ifstream aXmlFile;

    aXmlFile.open(i_sXmlFilePath, std::ios::in
#if defined(WNT) || defined(OS2)
                                          | std::ios::binary
#endif // WNT
    );

    if (! aXmlFile)
        return false;

    // Prepare buffer:
    aXmlFile.seekg(0, std::ios::end);
    unsigned long nBufferSize = (unsigned long) aXmlFile.tellg();
    o_rBuffer.SetSize(nBufferSize + 1);
    o_rBuffer.Data()[nBufferSize] = '\0';
    aXmlFile.seekg(0);

    // Read file:
    aXmlFile.read(o_rBuffer.Data(), (int) nBufferSize);
    bool ret = aXmlFile.good() != 0;
    aXmlFile.close();
    return ret;
}

