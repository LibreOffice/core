/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dht_stream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:41:36 $
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

#include <precomp.h>
#include <ary/doc/ht/dht_stream.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/ht/dht_html.hxx>
#include <ary/doc/ht/dht_link2ce.hxx>
#include <ary/doc/ht/dht_link2param.hxx>
#include <ary/doc/ht/dht_link2url.hxx>
#include <ary/doc/ht/dht_link2unknown.hxx>
#include <ary/doc/ht/dht_plain.hxx>
#include <ary/doc/ht/dht_white.hxx>



namespace ary
{
namespace doc
{
namespace ht
{


inline HyperText &
Stream::Out()
{
    return mrOut;
}




Stream::Stream( HyperText &         o_Receiver,
                E_TextMode          i_initialTextMode )
    :   mrOut(o_Receiver),
        mfCurTextFlusher( i_initialTextMode == tm_plain
                                ?   &Stream::PlainTextFlusher
                                :   &Stream::HtmlTextFlusher ),
        mText(300)
{
}

Stream::~Stream()
{

}

Stream &
Stream::operator<<( F_StreamModifier    i_modifier )
{
    (*i_modifier)(*this);
    return *this;
}

Stream &
Stream::operator<<( const char *    i_text )
{
    mText << i_text;
    return *this;
}

Stream &
Stream::operator<<( const String &  i_text )
{
    mText << i_text;
    return *this;
}

void
Stream::Add_Indentation( uintt i_count )
{
    Flush();
    Out().push_back(
                Indentation::Create_Component_(i_count) );
}

void
Stream::Add_Link2Ce( const String &      i_absolutePath,
                     const String &      i_display )
{
    Flush();
    Out().push_back(
                Link2Ce::Create_Component_(i_absolutePath, i_display) );
}

void
Stream::Add_Link2Unknown( const String &      i_type,
                          const String &      i_display )
{
    Flush();
    Out().push_back(
                Link2Unknown::Create_Component_(i_type, i_display) );
}

void
Stream::Add_Link2Url( const String &      i_url,
                      const String &      i_display)
{
    Flush();
    Out().push_back(
                Link2Url::Create_Component_(i_url, i_display) );
}

void
Stream::Add_Link2Param( const String &      i_parameter,
                        const String &      i_display )
{
    Flush();
    Out().push_back(
                Link2Param::Create_Component_(i_parameter, i_display) );
}

Stream::E_TextMode
Stream::CurrentMode() const
{
    return mfCurTextFlusher == &Stream::PlainTextFlusher
                ?   tm_plain
                :   tm_html;
}

void
Stream::Flush()
{
    if (mText.size() == 0)
        return;

    if (    mText.size() > 1
         OR *mText.c_str() != ' ')
    {
        (this->*mfCurTextFlusher)();
    }
    else
    {
        Out().push_back(Blank::Create_Component_());
    }

    mText.reset();
}

void
Stream::PlainTextFlusher()
{
    Out().push_back(
                PlainText::Create_Component_(String(mText.c_str())) );
}

void
Stream::HtmlTextFlusher()
{
    Out().push_back(
                HtmlText::Create_Component_(String(mText.c_str())) );
}


void
flush(Stream& io_stream)
{
    io_stream.Flush();
}

void
newline(Stream& io_stream)
{
    io_stream.Flush();
    io_stream.Out().push_back(NewParagraph::Create_Component_());
}

void
blank(Stream& io_stream)
{
    io_stream.mText << ' ';
}

void
plain(Stream& io_stream)
{
    io_stream.Flush();
    io_stream.mfCurTextFlusher = &Stream::PlainTextFlusher;
}

void
html(Stream& io_stream)
{
    io_stream.Flush();
    io_stream.mfCurTextFlusher = &Stream::HtmlTextFlusher;
}



}   // namespace ht
}   // namespace doc
}   // namespace ary
