/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_
#define CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_

#include "oslstream.hxx"
#include "filehelper.hxx"
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>

namespace configmgr { namespace localbe {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace io = css::io ;
namespace backend = css::configuration::backend ;

/**
  Class wrapping the use of the XOutputStream implementation on a file
  to make it handle a temporary file and synch the contents to the
  actual file being accessed only on successful completion of the output
  process.
  */
class LocalOutputStream : public cppu::WeakImplHelper1<io::XOutputStream>
{
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
            throw (backend::BackendAccessException, uno::RuntimeException) ;
        /** Destructor */
        ~LocalOutputStream(void) ;

        // closeOutput and mark as successful
        void finishOutput()
            throw (backend::BackendAccessException, uno::RuntimeException) ;
    protected :
        // XOutputStream
        virtual void SAL_CALL writeBytes(const uno::Sequence<sal_Int8>& aData)
            throw (io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException);

        virtual void SAL_CALL flush(void)
            throw (io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException);

        virtual void SAL_CALL closeOutput(void)
            throw (io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException) ;


    private :
        uno::Reference<io::XOutputStream> getOutputFile() ;

        /** Temporary file used during access */
        uno::Reference<io::XOutputStream> mTemporaryFile ;
        /** URL of the target file */
        rtl::OUString mFileUrl ;
        /** URL of the temporary file */
        rtl::OUString mTemporaryFileUrl ;
        /** File being written */
        osl::File *mWriteFile ;
} ;

} } // configmgr.localbe

#endif // CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_
