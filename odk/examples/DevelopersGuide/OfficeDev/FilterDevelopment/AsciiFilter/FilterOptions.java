/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

package FilterDevelopment.AsciiFilter;

import com.sun.star.uno.AnyConverter;
import com.sun.star.beans.PropertyValue;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.uno.Exception;
import com.sun.star.ucb.CommandAbortedException;
import com.sun.star.lang.IllegalArgumentException;

/*-************************************************************************
  @title        helper to analyze neccessary option properties of our filter
  @description  Our filter needs some neccessary properties for working:
                    - a stream for input or output
                    - or an URL for creating such streams
                    - informations about required action on filtering

  @attention    This class mustn't be threadsafe - because instances of it
                are used temp. only - not as members. So no concurrent access
                should occure.
                Another reason: It wuold be very difficult to safe every
                access on our internal member. To do so - we must implement
                special methods instead of allowing pure member access.
 ************************************************************************-*/

public class FilterOptions
{
    //_____________________________________
    // public member to provide these options to our outside filter class
    public  com.sun.star.io.XInputStream            m_xInput        ;
    public  com.sun.star.io.XOutputStream           m_xOutput       ;
    public  boolean                                 m_bStreamOwner  ;
    public  String                                  m_sURL          ;
    public  String                                  m_sOld          ;
    public  String                                  m_sNew          ;
    public  boolean                                 m_bCaseChange   ;
    public  boolean                                 m_bLower        ;

    //_____________________________________
    // private members for internal things
    private XMultiComponentFactory m_xMCF         ;
    private XComponentContext m_Ctx                ;

    //_____________________________________
    // interface
    /**
     * creates a new instance of this class
     * It use the given MediaDescriptor to find right
     * properties for initialization of the internal members.
     * To do so it use another interface method analyze()
     * which can be used after creation of an object instance
     * to set a new descriptor here.
     *
     * @param xSMGR
     *          we need it to create special help service top open
     *          streams in case they are not already a part of given
     *          MediaDescriptor
     *
     * @param bImport
     *          we must know which stream member should be valid initialized
     *
     * @param lDescriptor
     *          the initial MediaDescriptor to set internal member from it
     */
    public FilterOptions( XMultiComponentFactory                xMCF      ,
                          XComponentContext                     Context   ,
                          boolean                               bImport   ,
                          com.sun.star.beans.PropertyValue[]    lDescriptor )
    {
        m_xMCF = xMCF;
        m_Ctx = Context;
        analyze(bImport, lDescriptor);
    }

    /**
     * analyze given MediaDescriptor to find values for our internal member
     * It reset all members to defaults before - to prevent us against
     * mixed descriptor values!
     *
     * @param bImport
     *          we must know which stream member should be valid initialized
     *
     * @param lDescriptor
     *          the new MediaDescriptor to set internal member from it
     */
    public void analyze( boolean                            bImport     ,
                         com.sun.star.beans.PropertyValue[] lDescriptor )
    {
        m_xInput       = null        ;
        m_xOutput      = null        ;
        m_bStreamOwner = false       ;
        m_sURL         = null        ;
        m_sOld         = new String();
        m_sNew         = new String();
        m_bCaseChange  = false       ;
        m_bLower       = false       ;

        for ( int i=0; i<lDescriptor.length; ++i )
        {
            try
            {
                if (lDescriptor[i].Name.equals("FileName"))
                    m_sURL = AnyConverter.toString(lDescriptor[i].Value);
                else
                if (lDescriptor[i].Name.equals("InputStream"))
                    m_xInput = (com.sun.star.io.XInputStream)AnyConverter.toObject(new com.sun.star.uno.Type(com.sun.star.io.XInputStream.class), lDescriptor[i].Value);
                else
                if (lDescriptor[i].Name.equals("OutputStream"))
                    m_xOutput = (com.sun.star.io.XOutputStream)AnyConverter.toObject(new com.sun.star.uno.Type(com.sun.star.io.XOutputStream.class), lDescriptor[i].Value);
                else
                if (lDescriptor[i].Name.equals("FilterData"))
                {
                    com.sun.star.beans.PropertyValue[] lFilterProps = (com.sun.star.beans.PropertyValue[])AnyConverter.toArray(lDescriptor[i].Value);
                    int nCount = lFilterProps.length;
                    for (int p=0; p<nCount; ++p)
                    {
                        if (lFilterProps[p].Name.equals("OldString"))
                            m_sOld = AnyConverter.toString(lFilterProps[p].Value);
                        else
                        if (lFilterProps[p].Name.equals("NewString"))
                            m_sNew = AnyConverter.toString(lFilterProps[p].Value);
                        else
                        if (lFilterProps[p].Name.equals("LowerCase"))
                        {
                            m_bLower      = AnyConverter.toBoolean(lFilterProps[p].Value);
                            m_bCaseChange = true; // Set it after m_bLower - because an exception can occure and we must use default values then!
                        }
                    }
                }
            }
            catch(com.sun.star.lang.IllegalArgumentException exConvert)
            {
                // ONE argument has the wrong type
                // But I think we mustn't react here - because we setted
                // default values for every neccessary item we need.
                // In case this exception occures - this default exist
                // and we can live with it.
            }
        }

        // Decide if it's neccessary AND possible to open streams.
        // Outside user can check for valid FilterOptions by using
        // corresponding method isValid(). So it's not neccessary to
        // handle this error here in any case.
        if (m_xInput==null && m_xOutput==null && m_sURL!=null)
            impl_openStreams(bImport);
    }

    /**
     * with this method it's possible for the outside filter to decide
     * if he can use this FilterOptions realy or not.
     * That means especialy if neccessary streams are available or not.
     */
    public boolean isValid()
    {
        return(m_xInput!=null || m_xOutput!=null);
    }

    //_____________________________________
    // helper
    /**
     * In case we couldn't found any valid stream inside the given MediaDescriptor,
     * we must create it. Then we use a special helper service in combination
     * with an existing URL to open a stream for reading or writing. It depends
     * from given parameter bImport.
     *
     * Note: This method doesn't check for a valid URL. It must be done before.
     *
     * @param bImport
     *          inidcates which stream member must be valid as result of this call
     */
    private void impl_openStreams( boolean bImport )
    {
        try{
            com.sun.star.ucb.XSimpleFileAccess xHelper = (com.sun.star.ucb.XSimpleFileAccess)UnoRuntime.queryInterface(
                com.sun.star.ucb.XSimpleFileAccess.class,
                m_xMCF.createInstanceWithContext("com.sun.star.ucb.SimpleFileAccess", m_Ctx));
            if (xHelper!=null)
            {
                if (bImport==true)
                    m_xInput = xHelper.openFileRead(m_sURL);
                else
                    m_xOutput = xHelper.openFileWrite(m_sURL);
            }

            m_bStreamOwner = (m_xInput!=null || m_xOutput!=null);
        }
        catch(com.sun.star.ucb.CommandAbortedException exAborted) {}
        catch(com.sun.star.uno.Exception               exUno    ) {}
    }
}
