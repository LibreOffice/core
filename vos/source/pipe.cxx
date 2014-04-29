/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#include <vos/pipe.hxx>
#include <vos/diagnose.hxx>

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

    VOS_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");
    VOS_POSTCOND((*m_pPipeRef)(), "OPipe(): creation of pipe failed!\n");
}

/*****************************************************************************/
// OPipe()
/*****************************************************************************/

OPipe::OPipe( const rtl::OUString& strName,
              TPipeOption Options,
              const OSecurity& rSecurity)
{
    m_pPipeRef=
        new PipeRef(osl_createPipe(strName.pData,
                                   (oslPipeOptions)Options,
                                   (oslSecurity)rSecurity));

    VOS_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");
    VOS_POSTCOND((*m_pPipeRef)(), "OPipe(): creation of pipe failed!\n");
}

/*****************************************************************************/
// OPipe()
/*****************************************************************************/
OPipe::OPipe(const OPipe& pipe) :
OReference(), OObject()
{

    VOS_ASSERT(pipe.m_pPipeRef != 0);

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

    VOS_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");

    return (*m_pPipeRef)() != 0;
}

/*****************************************************************************/
// create
/*****************************************************************************/
sal_Bool OPipe::create( const rtl::OUString& strName,
                        TPipeOption Options,
                        const vos::OSecurity& rSecurity )
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
                                   (oslSecurity)rSecurity));

    VOS_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");

    return (*m_pPipeRef)() != 0;
}

/*****************************************************************************/
// operator=
/*****************************************************************************/
OPipe& OPipe::operator= (const OPipe& pipe)
{
    VOS_PRECOND(pipe.m_pPipeRef != 0, "OPipe::operator=: tried to assign an empty/invalid pipe\n");

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
    VOS_ASSERT(m_pPipeRef);
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
    VOS_ASSERT(pipe.m_pPipeRef != 0);

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

    VOS_POSTCOND(m_pPipeRef != 0, "OPipe(): new failed.\n");

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
    VOS_ASSERT(m_pPipeRef && (*m_pPipeRef)());

    /* loop until all desired bytes were read or an error occurred */
    sal_Int32 BytesRead= 0;
    sal_Int32 BytesToRead= n;
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receivePipe((*m_pPipeRef)(),
                                 pBuffer,
                                BytesToRead);

        /* error occurred? */
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
    VOS_ASSERT(m_pPipeRef && (*m_pPipeRef)());

    /* loop until all desired bytes were send or an error occurred */
    sal_Int32 BytesSend= 0;
    sal_Int32 BytesToSend= n;
    while (BytesToSend > 0)
    {
        sal_Int32 RetVal;

        RetVal= osl_sendPipe((*m_pPipeRef)(),
                                pBuffer,
                                BytesToSend);

        /* error occurred? */
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




