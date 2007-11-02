/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dht_stream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:04:35 $
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

#ifndef ARY_DOC_DHT_STREAM_HXX
#define ARY_DOC_DHT_STREAM_HXX

// USED SERVICES
#include <ary/doc/d_hypertext.hxx>



namespace ary
{
namespace doc
{
namespace ht
{
    class Stream;


typedef void (*     F_StreamModifier)(Stream&);

/// Causes the buffer in ->io_stream to be flushed.
void                flush(
                        Stream &            io_stream );
/// Adds a newline.
void                newline(
                        Stream &            io_stream );
/// Adds a single blank char.
void                blank(
                        Stream &            io_stream );
/// Modifies ->Stream to plain text mode.
void                plain(
                        Stream &            io_stream );
/// Modifies ->Stream to html text mode.
void                html(
                        Stream &            io_stream );




/** Interface to write items into a ->HyperText.

    Text can be intepreted as plain ASCII or as HTML. The default
    is plain ASCII.
    Also you can add links and can specify the link's label.
    If no label is provided for a link, the most specific part of the
    link's name will be used.


    @example
    using ary::doc::HyperText;
    using ary::doc::ht::Stream;

    Stream      aWriter;
    HyperText   aText;
    StreamGuard aGuard(aWriter,aText);

    Ce_id       nId = i_ceid;
    aWriter << "Hello," << newline;
    aWriter.Add_Link2Ce(GetAbsolutePath(nId), "SomeCodeEntity")
    aWriter << " will be transfered." << newline;
    aWriter Add_Link2Param("io_someParameter");
    aWriter << " shows something else."
            << flush;
*/
class Stream
{
  NON_COPYABLE(Stream);

  public:
    /// Marks the mode how documentation text is interpreted.
    enum E_TextMode
    {
        tm_plain,
        tm_html
    };

                        Stream(
                            HyperText &         o_Receiver,
                            E_TextMode          i_initialTextMode = tm_plain);
    /// Also flushes the stream to the connected ->HyperText, if any.
    virtual             ~Stream();

    Stream &            operator<<(
                            F_StreamModifier    i_modifier );
    /// Inserts text.
    Stream &            operator<<(
                            const char *        i_text );
    /// Inserts text.
    Stream &            operator<<(
                            const String &      i_text );

    void                Add_Indentation(
                            uintt               i_count );

    void                Add_Link2Ce(
                            const String &      i_absolutePath,
                            const String &      i_display = String::Null_() );
    void                Add_Link2Unknown(
                            const String &      i_type,
                            const String &      i_display = String::Null_() );
    void                Add_Link2Url(
                            const String &      i_url,
                            const String &      i_display = String::Null_() );
    void                Add_Link2Param(
                            const String &      i_parameter,
                            const String &      i_display = String::Null_() );

    E_TextMode          CurrentMode() const;

  private:
    friend void flush(Stream&);
    friend void newline(Stream&);
    friend void blank(Stream&);
    friend void plain(Stream&);
    friend void html(Stream&);

    // Locals
    typedef void (Stream::*F_TextFlusher)();

    void                Flush();
    void                PlainTextFlusher();
    void                HtmlTextFlusher();

    HyperText &         Out();

    // DATA
    HyperText &         mrOut;
    F_TextFlusher       mfCurTextFlusher;
    StreamStr           mText;
};



}   // namspace ht
}   // namspace doc
}   // namspace ary
#endif
