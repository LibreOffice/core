/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "lwpfilehdr.hxx"
#include "lwpdefs.hxx"

//Word Pro 96: 0x0a,
sal_uInt16 LwpFileHeader::m_nFileRevision = 0;

LwpFileHeader::LwpFileHeader()
    :m_nAppRevision(0),
    m_nAppReleaseNo(0), m_nRequiredAppRevision(0), m_nRequiredFileRevision(0),
    m_cDocumentID(), m_nRootIndexOffset(0)
{}
/**
 * @descr  read file header object from svstream
  */
sal_uInt32 LwpFileHeader::Read(LwpSvStream *pStrm)
{
    sal_uInt32 len = 0;
    *pStrm >> m_nAppRevision;
    len += sizeof(m_nAppRevision);
    *pStrm >> m_nFileRevision;
    len += sizeof(m_nFileRevision);
    *pStrm >> m_nAppReleaseNo;
    len += sizeof(m_nAppReleaseNo);
    *pStrm >> m_nRequiredAppRevision;
    len += sizeof(m_nRequiredAppRevision);
    *pStrm >> m_nRequiredFileRevision;
    len += sizeof(m_nRequiredFileRevision);
    len += m_cDocumentID.Read(pStrm);
    if (m_nFileRevision < 0x000B)
        m_nRootIndexOffset = BAD_OFFSET;
    else
    {
        *pStrm >> m_nRootIndexOffset;
        len += sizeof(m_nRootIndexOffset);
    }
    return len;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
