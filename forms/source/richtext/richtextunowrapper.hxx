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



#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX

#include <editeng/unotext.hxx>
#include <editeng/unoedsrc.hxx>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= IEngineTextChangeListener
    //====================================================================
    class IEngineTextChangeListener
    {
    public:
        virtual void potentialTextChange( ) = 0;
    };

    //====================================================================
    //= ORichTextUnoWrapper
    //====================================================================
    class ORichTextUnoWrapper : public SvxUnoText
    {
    public:
        ORichTextUnoWrapper( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener );

    protected:
        ~ORichTextUnoWrapper() throw();


    private:
        ORichTextUnoWrapper();                                          // never implemented
        ORichTextUnoWrapper( const ORichTextUnoWrapper& );              // never implemented
        ORichTextUnoWrapper& operator=( const ORichTextUnoWrapper& );   // never implemented
    };

    //====================================================================
    //= RichTextEditSource
    //====================================================================
    class RichTextEditSource : public SvxEditSource
    {
    private:
        EditEngine&                 m_rEngine;
        SvxTextForwarder*           m_pTextForwarder;
        IEngineTextChangeListener*  m_pTextChangeListener;

    public:
        RichTextEditSource( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener );

        // SvxEditSource
        virtual SvxEditSource*      Clone() const;
        virtual SvxTextForwarder*   GetTextForwarder();
        virtual void                UpdateData();

    protected:
        ~RichTextEditSource();

    private:
        RichTextEditSource( );                                      // never implemented
        RichTextEditSource( const RichTextEditSource& _rSource );   // never implemented
        RichTextEditSource& operator=( const RichTextEditSource& ); // never implemented
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX

