/*************************************************************************
 *
 *  $RCSfile: pipe.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:16 $
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


#include <vos/pipe.hxx>
#include <vos/diagnose.hxx>

#ifdef _USE_NAMESPACE
using namespace vos;
#endif


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
OPipe::OPipe(const OPipe& pipe)
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
        osl_destroyPipe((*m_pPipeRef)());
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
                        const NAMESPACE_VOS(OSecurity)& rSecurity )
{
    // if this was a valid pipe, decrease reference
    if ((m_pPipeRef) && (m_pPipeRef->release() == 0))
    {
        osl_destroyPipe((*m_pPipeRef)());
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
        osl_destroyPipe((*m_pPipeRef)());
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
        osl_destroyPipe((*m_pPipeRef)());
        delete m_pPipeRef;
    }
    m_pPipeRef= 0;
}

/*****************************************************************************/
// accept
/*****************************************************************************/
OPipe::TPipeError OPipe::accept(OStreamPipe& Connection)
{
    VOS_ASSERT(m_pPipeRef && (*m_pPipeRef)());

    Connection= osl_acceptPipe((*m_pPipeRef)());

    if(Connection.isValid())
        return E_None;
    else
        return getError();
}

/*****************************************************************************/
// recv
/*****************************************************************************/
sal_Int32 OPipe::recv(void* pBuffer, sal_uInt32 BytesToRead)
{
    VOS_ASSERT(m_pPipeRef && (*m_pPipeRef)());

    return osl_receivePipe((*m_pPipeRef)(),
                            pBuffer,
                           BytesToRead);

}

/*****************************************************************************/
// send
/*****************************************************************************/
sal_Int32 OPipe::send(const void* pBuffer, sal_uInt32 BytesToSend)
{
    VOS_ASSERT(m_pPipeRef && (*m_pPipeRef)());

    return osl_sendPipe((*m_pPipeRef)(),
                          pBuffer,
                          BytesToSend);

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
OStreamPipe::OStreamPipe(const OStreamPipe& pipe)
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
        osl_destroyPipe((*m_pPipeRef)());
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
    VOS_ASSERT(m_pPipeRef && (*m_pPipeRef)());

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




