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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
