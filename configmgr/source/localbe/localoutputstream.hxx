/*************************************************************************
 *
 *  $RCSfile: localoutputstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2002-06-11 15:29:56 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_
#define CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_

#ifndef _CONFIGMGR_OSLSTREAM_HXX_
#include "oslstream.hxx"
#endif // _CONFIGMGR_OSLSTREAM_HXX_

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif // _CONFIGMGR_FILEHELPER_HXX_

namespace configmgr {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace io = css::io ;

/**
  Class wrapping the use of the XOutputStream implementation on a file
  to make it handle a temporary file and synch the contents to the
  actual file being accessed only on successful completion of the output
  process.
  */
class LocalOutputStream : public cppu::WeakImplHelper1<io::XOutputStream> {
    public :
        /**
          Constructor using the URL of the file to access.
          The actual writing will occur in a temporary file
          whose name is derived from the URL, and the file
          specified by the parameter will be overwritten
          on closing the stream.

          @param aFileUrl   URL of the file to write
          @throws   com::sun::star::io::IOException
                    if access to the temporary file fails.
          */
        LocalOutputStream(const rtl::OUString& aFileUrl)
            throw (io::IOException) ;
        /** Destructor */
        ~LocalOutputStream(void) ;

        // XOutputStream
        virtual void SAL_CALL writeBytes(const uno::Sequence<sal_Int8>& aData)
            throw (io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException) {
                mTemporaryFile->writeBytes(aData) ;
            }
        virtual void SAL_CALL flush(void)
            throw (io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException) {
                mTemporaryFile->flush() ;
            }
        virtual void SAL_CALL closeOutput(void)
            throw (io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException) ;

    protected :

    private :
        /** Temporary file used during access */
        uno::Reference<io::XOutputStream> mTemporaryFile ;
        /** URL of the target file */
        rtl::OUString mFileUrl ;
        /** URL of the temporary file */
        rtl::OUString mTemporaryFileUrl ;
        /** Has the stream been closed yet? */
        sal_Bool bIsClosed ;
        /** File being written */
        osl::File *mWriteFile ;
} ;

} // configmgr

#endif // CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_
