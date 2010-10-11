/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <vos/pipe.hxx>
#include <osl/diagnose.h>

using namespace vos;

///////////////////////////////////////////////////////////////////////////////
// Pipe


VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OPipe, vos),
                        VOS_NAMESPACE(OPipe, vos),
                        VOS_NAMESPACE(OObject, vos), 0);

/*****************************************************************************/
// OPipe()
/*****************************************************************************/
OPipe::OPipe()
{
    m_pPipeRef= 0;
}

/*****************************************************************************/
// OPipe()
/*****************************************************************************/

OPipe::OPipe( const rtl::OUString& strName, TPipeOption Options)
{
    m_pPipeRef =
        new PipeRef( osl_createPipe(strName.pData,
                                    (oslPipeOptions)Options,
                                    NULL) );

    OSL_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");
    OSL_POSTCOND((*m_pPipeRef)(), "OPipe(): creation of pipe failed!\n");
}

/*****************************************************************************/
// OPipe()
/*****************************************************************************/

OPipe::OPipe( const rtl::OUString& strName,
              TPipeOption Options,
              const osl::Security& rSecurity)
{
    m_pPipeRef=
        new PipeRef(osl_createPipe(strName.pData,
                                   (oslPipeOptions)Options,
                                   rSecurity.getHandle()));

    OSL_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");
    OSL_POSTCOND((*m_pPipeRef)(), "OPipe(): creation of pipe failed!\n");
}

/*****************************************************************************/
// OPipe()
/*****************************************************************************/
OPipe::OPipe(const OPipe& pipe) :
OReference(), OObject()
{

    OSL_ASSERT(pipe.m_pPipeRef != 0);

    m_pPipeRef= pipe.m_pPipeRef;

    m_pPipeRef->acquire();
}

/*****************************************************************************/
// OPipe()
/*****************************************************************************/
OPipe::OPipe(oslPipe Pipe)
{
    m_pPipeRef = new PipeRef(Pipe);
}


/*****************************************************************************/
// ~OPipe()
/*****************************************************************************/
OPipe::~OPipe()
{
    close();
}

/*****************************************************************************/
// create
/*****************************************************************************/
sal_Bool OPipe::create( const rtl::OUString& strName, TPipeOption Options )
{
    // if this was a valid pipe, decrease reference
    if ((m_pPipeRef) && (m_pPipeRef->release() == 0))
    {
        osl_releasePipe((*m_pPipeRef)());
        delete m_pPipeRef;
        m_pPipeRef= 0;
    }

    m_pPipeRef=
        new PipeRef(osl_createPipe(strName.pData,
                                   (oslPipeOptions)Options,
                                   NULL));

    OSL_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");

    return (*m_pPipeRef)() != 0;
}

/*****************************************************************************/
// create
/*****************************************************************************/
sal_Bool OPipe::create( const rtl::OUString& strName,
                        TPipeOption Options,
                        const osl::Security& rSecurity )
{
    // if this was a valid pipe, decrease reference
    if ((m_pPipeRef) && (m_pPipeRef->release() == 0))
    {
        osl_releasePipe((*m_pPipeRef)());
        delete m_pPipeRef;
        m_pPipeRef= 0;
    }

    m_pPipeRef=
        new PipeRef(osl_createPipe(strName.pData,
                                   (oslPipeOptions)Options,
                                   rSecurity.getHandle()));

    OSL_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");

    return (*m_pPipeRef)() != 0;
}

/*****************************************************************************/
// operator=
/*****************************************************************************/
OPipe& OPipe::operator= (const OPipe& pipe)
{
    OSL_PRECOND(pipe.m_pPipeRef != 0, "OPipe::operator=: tried to assign an empty/invalid pipe\n");

    if (m_pPipeRef == pipe.m_pPipeRef)
        return *this;

    // if this was a valid pipe, decrease reference
    if ((m_pPipeRef) && (m_pPipeRef->release() == 0))
    {
        osl_releasePipe((*m_pPipeRef)());
        delete m_pPipeRef;
        m_pPipeRef= 0;
    }

    m_pPipeRef= pipe.m_pPipeRef;

    m_pPipeRef->acquire();

    return *this;
}

/*****************************************************************************/
// operator oslPipe()
/*****************************************************************************/
OPipe::operator oslPipe() const
{
    OSL_ASSERT(m_pPipeRef);
    return (*m_pPipeRef)();
}

/*****************************************************************************/
// isValid()
/*****************************************************************************/
sal_Bool OPipe::isValid() const
{
    return m_pPipeRef != 0 && (*m_pPipeRef)() != 0;
}


/*****************************************************************************/
// close
/*****************************************************************************/
void OPipe::close()
{
    if (m_pPipeRef && (m_pPipeRef->release() == 0))
    {
        osl_releasePipe((*m_pPipeRef)());
        delete m_pPipeRef;
    }
    m_pPipeRef= 0;
}

/*****************************************************************************/
// accept
/*****************************************************************************/
OPipe::TPipeError OPipe::accept(OStreamPipe& Connection)
{
    if ( isValid() )
    {
        Connection = osl_acceptPipe((*m_pPipeRef)());

        if(Connection.isValid())
            return E_None;
    }

    return getError();
}

/*****************************************************************************/
// recv
/*****************************************************************************/
sal_Int32 OPipe::recv(void* pBuffer, sal_uInt32 BytesToRead)
{
    if ( isValid() )
        return osl_receivePipe((*m_pPipeRef)(),
                             pBuffer,
                            BytesToRead);
    else
        return -1;

}

/*****************************************************************************/
// send
/*****************************************************************************/
sal_Int32 OPipe::send(const void* pBuffer, sal_uInt32 BytesToSend)
{
    if ( isValid() )
        return osl_sendPipe((*m_pPipeRef)(),
                            pBuffer,
                            BytesToSend);
    else
        return -1;
}

/*****************************************************************************/
// getError
/*****************************************************************************/
OPipe::TPipeError OPipe::getError() const
{
    if (m_pPipeRef)
        return (TPipeError)osl_getLastPipeError((*m_pPipeRef)());
    else
        return (TPipeError)osl_getLastPipeError(NULL);
}



VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OStreamPipe, vos),
                        VOS_NAMESPACE(OStreamPipe, vos),
                        VOS_NAMESPACE(OPipe, vos), 0);



/*****************************************************************************/
// OStreamPipe
/*****************************************************************************/
OStreamPipe::OStreamPipe()
{
}

/*****************************************************************************/
// OStreamPipe
/*****************************************************************************/
OStreamPipe::OStreamPipe(oslPipe Pipe) :
    OPipe(Pipe)
{
}

/*****************************************************************************/
// OStreamPipe
// copy constructor
/*****************************************************************************/
OStreamPipe::OStreamPipe(const OStreamPipe& pipe) :
OPipe(), IStream()
{
    OSL_ASSERT(pipe.m_pPipeRef != 0);

    m_pPipeRef= pipe.m_pPipeRef;

    m_pPipeRef->acquire();
}

/*****************************************************************************/
// ~OStreamPipe
/*****************************************************************************/
OStreamPipe::~OStreamPipe()
{
}

/*****************************************************************************/
// operator=(oslPipe)
/*****************************************************************************/
OStreamPipe& OStreamPipe::operator=(oslPipe Pipe)
{

    // if this was a valid pipe, decrease reference
    if (m_pPipeRef && (m_pPipeRef->release() == 0))
    {
        osl_releasePipe((*m_pPipeRef)());
        delete m_pPipeRef;
        m_pPipeRef= 0;
    }

    m_pPipeRef= new PipeRef(Pipe);

    OSL_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");

    return *this;
}

/*****************************************************************************/
// operator=OPipe
/*****************************************************************************/

OStreamPipe& OStreamPipe::operator= (const OPipe& pipe)
{
    OPipe::operator= ( pipe );
    return *this;
}

/*****************************************************************************/
// read
/*****************************************************************************/
sal_Int32 OStreamPipe::read(void* pBuffer, sal_uInt32 n) const
{
    OSL_ASSERT(m_pPipeRef && (*m_pPipeRef)());

    /* loop until all desired bytes were read or an error occured */
    sal_Int32 BytesRead= 0;
    sal_Int32 BytesToRead= n;
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receivePipe((*m_pPipeRef)(),
                                 pBuffer,
                                BytesToRead);

        /* error occured? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToRead -= RetVal;
        BytesRead += RetVal;
        pBuffer= (sal_Char*)pBuffer + RetVal;
    }

    return BytesRead;
}

/*****************************************************************************/
// write
/*****************************************************************************/
sal_Int32 OStreamPipe::write(const void* pBuffer, sal_uInt32 n)
{
    OSL_ASSERT(m_pPipeRef && (*m_pPipeRef)());

    /* loop until all desired bytes were send or an error occured */
    sal_Int32 BytesSend= 0;
    sal_Int32 BytesToSend= n;
    while (BytesToSend > 0)
    {
        sal_Int32 RetVal;

        RetVal= osl_sendPipe((*m_pPipeRef)(),
                                pBuffer,
                                BytesToSend);

        /* error occured? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToSend -= RetVal;
        BytesSend += RetVal;
        pBuffer= (sal_Char*)pBuffer + RetVal;
    }

    return BytesSend;
}

sal_Bool OStreamPipe::isEof() const
{
    return isValid();
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
