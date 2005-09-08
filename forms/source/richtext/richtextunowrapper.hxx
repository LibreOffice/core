/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: richtextunowrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:09:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX

#ifndef _SVX_UNOTEXT_HXX
#include <svx/unotext.hxx>
#endif
#ifndef _SVX_UNOEDSRC_HXX
#include <svx/unoedsrc.hxx>
#endif

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

