/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: systemshell.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:34:41 $
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

#ifndef _SYSTEMSHELL_HXX_
#define _SYSTEMSHELL_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace SystemShell
{
    /** Add a file to the system shells recent document list if there is any.
          This function may have no effect under Unix because there is no
          standard API among the different desktop managers.

          @param aFileUrl
                    The file url of the document.

          @param aMimeType
          The mime content type of the document specified by aFileUrl.
          If an empty string will be provided "application/octet-stream"
          will be used.
    */
    void AddToRecentDocumentList(const rtl::OUString& aFileUrl, const rtl::OUString& aMimeType);
}

#endif
