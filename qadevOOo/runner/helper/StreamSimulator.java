/*************************************************************************
 *
 *  $RCSfile: StreamSimulator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2003-11-18 16:14:54 $
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

package helper;

import com.sun.star.uno.UnoRuntime;


import com.sun.star.lang.DisposedException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.Exception;
import com.sun.star.ucb.XSimpleFileAccess;

/**
 * It simulates an input and output stream and
 * implements the interfaces XInputStream, XOutputStream.
 * So it can be used for testing loading/saving of documents
 * using streams instead of URLs.
 *
 */
public class StreamSimulator implements com.sun.star.io.XInputStream    ,
                                        com.sun.star.io.XOutputStream   ,
                                        com.sun.star.io.XSeekable
{
    //_________________________________
    /**
     * @member  m_sFileName     name of the corrsponding file on disk
     * @member  m_xInStream     the internal input stream for reading
     * @member  m_xOutStream    the internal input stream for writing
     * @member  m_xSeek         points at runtime to m_xInStream or m_xOutStream and make it seekable
     *
     * @member  //m_aProtocol     the external set protocol object for logging messages
     * @member  m_bInWasUsed    indicates, that the input stream interface was used
     * @member  m_bOutWasUsed   indicates, that the output stream interface was used
     */

    private String                          m_sFileName     ;
    private com.sun.star.io.XInputStream    m_xInStream     ;
    private com.sun.star.io.XOutputStream   m_xOutStream    ;
    private com.sun.star.io.XSeekable       m_xSeek         ;

    //public  ComplexTestEnvironment          //m_aProtocol     ;
    public  boolean                         m_bInWasUsed    ;
    public  boolean                         m_bOutWasUsed   ;

    //_________________________________
    /**
     * construct a new instance of this class
     * It set the name of the correspojnding file on disk, which
     * should be source or target for the following operations on
     * this object. And it regulate if it should function as
     * input or output stream.
     *
     * @param   sFileName
     *              name of the file on disk
     *              Will be used as source (if param bInput==true)
     *              or as target (if param bInput==false).
     *
     * @param   bInput
     *              it specify, which interface should work at this object.
     *              <TRUE/>  => we simulate an input stream
     *              <FALSE/> => we simulate an output stream
     *
     * @throw   com.sun.star.io.NotConnectedException
     *              in case the internal streams to the file on disk couldn't established.
     *              They are neccessary. Otherwhise this simulator can't realy work.
     */
    public StreamSimulator( String  sFileName , boolean bInput ,
        lib.TestParameters param   ) throws com.sun.star.io.NotConnectedException
    {
        ////m_aProtocol = new ComplexTestEnvironment();
        m_sFileName     = sFileName ;
        m_bInWasUsed    = false     ;
        m_bOutWasUsed   = false     ;

        try
        {
            XSimpleFileAccess xHelper = (XSimpleFileAccess)
                UnoRuntime.queryInterface(XSimpleFileAccess.class,
                    ((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.ucb.SimpleFileAccess"));
/*            com.sun.star.ucb.XSimpleFileAccess xHelper = (com.sun.star.ucb.XSimpleFileAccess)OfficeConnect.createRemoteInstance(
                com.sun.star.ucb.XSimpleFileAccess.class,
                "com.sun.star.ucb.SimpleFileAccess");*/

            if (xHelper == null)
                throw new com.sun.star.io.NotConnectedException("ucb helper not available. Can't create streams.");

            if (bInput)
            {
                m_xInStream = xHelper.openFileRead(m_sFileName);
                m_xSeek = (com.sun.star.io.XSeekable)UnoRuntime.queryInterface(
                            com.sun.star.io.XSeekable.class,
                            m_xInStream);
            }
            else
            {
                m_xOutStream = xHelper.openFileWrite(m_sFileName);
                m_xSeek = (com.sun.star.io.XSeekable)UnoRuntime.queryInterface(
                            com.sun.star.io.XSeekable.class,
                            m_xOutStream);
            }
        }
        catch(com.sun.star.uno.Exception exUno)
        {
            ////m_aProtocol.log("\tstream not open. throw NotConnectedException\n\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("Could not open the file.");
        }
    }

/*    public void finalize()
    {
        ////m_aProtocol.log("finalize was called. Please check if it was right or not.\n");
    } */

    //_________________________________
    /**
     * following methods simulates the XInputStream.
     * The notice all actions inside the internal protocol
     * and try to map all neccessary functions to the internal
     * open in-stream.
     */
    public int readBytes( /*OUT*/ byte[][] lData        ,
                          /*IN*/  int      nBytesToRead ) throws com.sun.star.io.NotConnectedException      ,
                                                                 com.sun.star.io.BufferSizeExceededException,
                                                                 com.sun.star.io.IOException
    {
        //m_aProtocol.log("readBytes(lData["+lData.length+"]["+lData[0]+"],"+nBytesToRead+")\n{\n");
        m_bInWasUsed = true;

        if (m_xInStream == null)
        {
            //m_aProtocol.log("\tstream not open. throw NotConnectedException\n\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        int nRead = 0;
        try
        {
            nRead = m_xInStream.readBytes(lData,nBytesToRead);
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) { //m_aProtocol.log("\tgot NotConnectedException\n\tfailed\n}\n"      ); throw exConnect;
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) { //m_aProtocol.log("\tgot BufferSizeExceededException\n\tfailed\n}\n"); throw exBuffer;
        }
        catch (com.sun.star.io.IOException                 exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"                ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"           ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception                  exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"                  );
        }

        //m_aProtocol.log("\treads "+nRead+" bytes\n\tOK\n}\n");

        //if (nRead != nBytesToRead)
            //m_aProtocol.log("there are some missing bytes for reading!\n");

        return nRead;
    }

    //_________________________________

    public int readSomeBytes( /*OUT*/ byte[][] lData           ,
                              /*IN*/  int      nMaxBytesToRead ) throws com.sun.star.io.NotConnectedException       ,
                                                                        com.sun.star.io.BufferSizeExceededException ,
                                                                        com.sun.star.io.IOException
    {
        //m_aProtocol.log("readSomeBytes(lData["+lData.length+"]["+lData[0]+"],"+nMaxBytesToRead+")\n{\n");
        m_bInWasUsed = true;

        if (m_xInStream == null)
        {
            //m_aProtocol.log("\tstream not open. throw NotConnectedException\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        int nRead = 0;
        try
        {
            nRead = m_xInStream.readSomeBytes(lData,nMaxBytesToRead);
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) { //m_aProtocol.log("\tgot NotConnectedException\n\tfailed\n}\n"      ); throw exConnect;
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) { //m_aProtocol.log("\tgot BufferSizeExceededException\n\tfailed\n}\n"); throw exBuffer;
        }
        catch (com.sun.star.io.IOException                 exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"                ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"           ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception                  exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"                  );
        }

        //m_aProtocol.log("\treads "+nRead+" bytes\n\tOK\n}\n");

        //if (nRead != nMaxBytesToRead)
            //m_aProtocol.log("there are some missing bytes for reading!");

        return nRead;
    }

    //_________________________________

    public void skipBytes( /*IN*/ int nBytesToSkip ) throws com.sun.star.io.NotConnectedException       ,
                                                            com.sun.star.io.BufferSizeExceededException ,
                                                            com.sun.star.io.IOException
    {
        //m_aProtocol.log("skipBytes("+nBytesToSkip+")\n{\n");
        m_bInWasUsed = true;

        if (m_xInStream == null)
        {
            //m_aProtocol.log("\tstream not open. throw NotConnectedException\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xInStream.skipBytes(nBytesToSkip);
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) { //m_aProtocol.log("\tgot NotConnectedException\n\tfailed\n}\n"      ); throw exConnect;
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) { //m_aProtocol.log("\tgot BufferSizeExceededException\n\tfailed\n}\n"); throw exBuffer;
        }
        catch (com.sun.star.io.IOException                 exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"                ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"           ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception                  exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"                  );
        }

        //m_aProtocol.log("\tOK\n}\n");
    }

    //_________________________________

    public int available() throws com.sun.star.io.NotConnectedException,
                                  com.sun.star.io.IOException
    {
        //m_aProtocol.log("available()\n{\n");
        m_bInWasUsed = true;

        if (m_xInStream == null)
        {
            //m_aProtocol.log("\tstream not open. throw NotConnectedException\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        int nAvailable = 0;
        try
        {
            nAvailable = m_xInStream.available();
        }
        catch (com.sun.star.io.NotConnectedException exConnect) { //m_aProtocol.log("\tgot NotConnectedException\n\tfailed\n}\n"); throw exConnect;
        }
        catch (com.sun.star.io.IOException           exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"          ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException     exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"     ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception            exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"            );
        }

        //m_aProtocol.log("\treturns "+nAvailable+" bytes\n\tOK\n}\n");
        return nAvailable;
    }

    //_________________________________

    public void closeInput() throws com.sun.star.io.NotConnectedException,
                                    com.sun.star.io.IOException
    {
        //m_aProtocol.log("closeInput()\n{\n");
        m_bInWasUsed = true;

        if (m_xInStream == null)
        {
            //m_aProtocol.log("\tstream not open. throw NotConnectedException\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xInStream.closeInput();
        }
        catch (com.sun.star.io.NotConnectedException exConnect) { //m_aProtocol.log("\tgot NotConnectedException\n\tfailed\n}\n"); throw exConnect;
        }
        catch (com.sun.star.io.IOException           exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"          ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException     exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"     ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception            exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"            );
        }

        //m_aProtocol.log("\tOK\n}\n");
    }

    //_________________________________
    /**
     * following methods simulates the XOutputStream.
     * The notice all actions inside the internal protocol
     * and try to map all neccessary functions to the internal
     * open out-stream.
     */
    public void writeBytes( /*IN*/byte[] lData ) throws com.sun.star.io.NotConnectedException       ,
                                                        com.sun.star.io.BufferSizeExceededException ,
                                                        com.sun.star.io.IOException
    {
        //m_aProtocol.log("writeBytes(lData["+lData.length+"])\n{\n");
        m_bOutWasUsed = true;

        if (m_xOutStream == null)
        {
            //m_aProtocol.log("\tstream not open. throw NotConnectedException\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xOutStream.writeBytes(lData);
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) { //m_aProtocol.log("\tgot NotConnectedException\n\tfailed\n}\n"      ); throw exConnect;
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) { //m_aProtocol.log("\tgot BufferSizeExceededException\n\tfailed\n}\n"); throw exBuffer;
        }
        catch (com.sun.star.io.IOException                 exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"                ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"           ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception                  exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"                  );
        }

        //m_aProtocol.log("\tOK\n}\n");
    }

    //_________________________________

    public void flush() throws com.sun.star.io.NotConnectedException        ,
                               com.sun.star.io.BufferSizeExceededException  ,
                               com.sun.star.io.IOException
    {
        //m_aProtocol.log("flush()\n{\n");
        m_bOutWasUsed = true;

        if (m_xOutStream == null)
        {
            //m_aProtocol.log("\tstream not open. throw NotConnectedException\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xOutStream.flush();
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) { //m_aProtocol.log("\tgot NotConnectedException\n\tfailed\n}\n"      ); throw exConnect;
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) { //m_aProtocol.log("\tgot BufferSizeExceededException\n\tfailed\n}\n"); throw exBuffer;
        }
        catch (com.sun.star.io.IOException                 exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"                ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"           ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception                  exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"                  );
        }
        //m_aProtocol.log("\tOK\n}\n");
    }

    //_________________________________

    public void closeOutput() throws com.sun.star.io.NotConnectedException      ,
                                     com.sun.star.io.BufferSizeExceededException,
                                     com.sun.star.io.IOException
    {
        //m_aProtocol.log("closeOutput()\n{\n");
        m_bOutWasUsed = true;

        if (m_xOutStream == null)
        {
            //m_aProtocol.log("\tstream not open. throw NotConnectedException\n\tfailed\n}\n");
            throw new com.sun.star.io.NotConnectedException("stream not open");
        }

        try
        {
            m_xOutStream.closeOutput();
        }
        catch (com.sun.star.io.NotConnectedException       exConnect) { //m_aProtocol.log("\tgot NotConnectedException\n\tfailed\n}\n"      ); throw exConnect;
        }
        catch (com.sun.star.io.BufferSizeExceededException exBuffer ) { //m_aProtocol.log("\tgot BufferSizeExceededException\n\tfailed\n}\n"); throw exBuffer;
        }
        catch (com.sun.star.io.IOException                 exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"                ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException           exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"           ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception                  exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"                  );
        }

        //m_aProtocol.log("\tOK\n}\n");
    }

    //_________________________________
    /**
     * following methods simulates the XSeekable.
     * The notice all actions inside the internal protocol
     * and try to map all neccessary functions to the internal
     * open stream.
     */
    public void seek( /*IN*/long nLocation ) throws com.sun.star.lang.IllegalArgumentException,
                                                    com.sun.star.io.IOException
    {
        //m_aProtocol.log("seek("+nLocation+")\n{\n");

        if (m_xInStream != null)
            m_bInWasUsed = true;
        else
        if (m_xOutStream != null)
            m_bOutWasUsed = true;
        else
            //m_aProtocol.log("\tno stream open!\n");

        if (m_xSeek == null)
        {
            //m_aProtocol.log("\tstream not seekable. throw IOException\n\tfailed\n}\n");
            throw new com.sun.star.io.IOException("stream not seekable");
        }

        try
        {
            m_xSeek.seek(nLocation);
        }
        catch (com.sun.star.lang.IllegalArgumentException exArg    ) { //m_aProtocol.log("\tgot IllegalArgumentException\n\tfailed\n}\n" ); throw exArg;
        }
        catch (com.sun.star.io.IOException                exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"              ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException          exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"         ); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception                 exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"                );
        }

        //m_aProtocol.log("\tOK\n}\n");
    }

    //_________________________________

    public long getPosition() throws com.sun.star.io.IOException
    {
        //m_aProtocol.log("getPosition()\n{\n");

        if (m_xInStream != null)
            m_bInWasUsed = true;
        else
        if (m_xOutStream != null)
            m_bOutWasUsed = true;
        else
            //m_aProtocol.log("\tno stream open!\n");

        if (m_xSeek == null)
        {
            //m_aProtocol.log("\tstream not seekable. throw IOException\n\tfailed\n}\n");
            throw new com.sun.star.io.IOException("stream not seekable");
        }

        long nPos = 0;
        try
        {
            nPos = m_xSeek.getPosition();
        }
        catch (com.sun.star.io.IOException       exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"     ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception        exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"       );
        }

        //m_aProtocol.log("\treturns pos="+nPos+"\n\tOK\n}\n");
        return nPos;
    }

    //_________________________________

    public long getLength() throws com.sun.star.io.IOException
    {
        //m_aProtocol.log("getLength()\n{\n");

        if (m_xInStream != null)
            m_bInWasUsed = true;
        else
        if (m_xOutStream != null)
            m_bOutWasUsed = true;
        else
            //m_aProtocol.log("\tno stream open!\n");

        if (m_xSeek == null)
        {
            //m_aProtocol.log("\tstream not seekable. throw IOException\n\tfailed\n}\n");
            throw new com.sun.star.io.IOException("stream not seekable");
        }

        long nLen = 0;
        try
        {
            nLen = m_xSeek.getLength();
        }
        catch (com.sun.star.io.IOException       exIO     ) { //m_aProtocol.log("\tgot IOException\n\tfailed\n}\n"     ); throw exIO;
        }
        catch (com.sun.star.uno.RuntimeException exRuntime) { //m_aProtocol.log("\tgot RuntimeException\n\tfailed\n}\n"); throw exRuntime;
        }
        catch (com.sun.star.uno.Exception        exUno    ) { //m_aProtocol.log("\tgot Exception\n\tfailed\n}\n"       );
        }

        //m_aProtocol.log("\treturns len="+nLen+"\n\tOK\n}\n");
        return nLen;
    }
}
