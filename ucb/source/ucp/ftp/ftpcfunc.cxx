/*************************************************************************
 *
 *  $RCSfile: ftpcfunc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: abi $ $Date: 2002-08-28 07:29:49 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <string.h>

#include "ftpcontentidentifier.hxx"
#include "ftpinpstr.hxx"

using namespace ftp;
using namespace com::sun::star::uno;

extern "C" {

    int ftp_write(void *buffer,size_t size,size_t nmemb,void *stream)
    {
        FTPStreamContainer *_stream =
            reinterpret_cast<FTPStreamContainer*>(stream);

        if(!_stream)
            return 0;

        return _stream->write(buffer,size,nmemb);
    }


//      int write2InputStream(void *buffer,size_t size,size_t nmemb,void *stream)
//      {
//          size_t ret = size*nmemb;
//          if(!(stream && ret))   // OK, no error if nothing can be written.
//              return ret;

//          FTPBufferContainer *p =
//              reinterpret_cast<FTPBufferContainer*>(stream);
//          if(p && p->m_out)
//              p->m_out->append(buffer,size,nmemb);
//          return ret;
//      }


//      /** Callback for curl_easy_perform(),
//       *  forwarding the written content to the outputstream.
//       */

//      int write2OutputStream(void *buffer,size_t size,size_t nmemb,void *stream)
//      {
//          size_t ret = size*nmemb;

//          if(!(stream && ret))  // OK, no error if nothing can be written.
//              return ret;

//          try{
//              FTPStreamContainer *p =
//                  reinterpret_cast<FTPStreamContainer*>(stream);
//              if(p && p->m_out.is())
//                  p->m_out->writeBytes(
//                      Sequence<sal_Int8>(static_cast<sal_Int8*>(buffer),
//                                         size*nmemb)
//                  );
//              return ret;
//          } catch(const Exception&) {
//              return 0;
//          }
//      }


    int ftp_passwd(void *client,char*prompt,char*buffer,int bufferlength)
    {
//          FTPClient *p =
//              reinterpret_cast<FTPClient>(client);

//          // 'passwd' returns actually "username:password"
//          rtl::OUString passwd = p->passwd();
//          rtl::OString opasswd(passwd.getStr(),
//                               passwd.getLength(),
//                               RTL_TEXTENCODING_UTF8);
//          if(strlen(opasswd.getStr()) <= bufferlength)
//              strncpy(buffer,opasswd.getStr(),opasswd.getLength());
//          else
//      strcpy(buffer,opasswd.getStr());
        strcpy(buffer,"psswd");

        return 0;
    }


}
