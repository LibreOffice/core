/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contentreader.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:35:46 $
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

#ifndef CONTENTREADER_HXX_INCLUDED
#define CONTENTREADER_HXX_INCLUDED

#ifndef BASEREADER_HXX_INCLUDED
#include "internal/basereader.hxx"
#endif

class ITag;

class CContentReader : public CBaseReader
{
public:
    virtual ~CContentReader();

    //CContentReader( const std::string& DocumentName );
    CContentReader( const std::string& DocumentName, LocaleSet_t const & DocumentLocale );

    /** Get the chunkbuffer.

        @return
        the chunkbuffer of the document.
    */
    inline ChunkBuffer_t const & getChunkBuffer( ) const{ return m_ChunkBuffer; };

protected: // protected because its only an implementation relevant class

    /** start_element occurs when a tag is start.

        @param raw_name
        raw name of the tag.
        @param local_name
        local name of the tag.
        @param attributes
        attribute structure.
    */
    virtual void start_element(
        const std::wstring& raw_name,
        const std::wstring& local_name,
        const XmlTagAttributes_t& attributes);

    /** end_element occurs when a tag is closed

        @param raw_name
        raw name of the tag.
        @param local_name
        local name of the tag.
    */
    virtual void end_element(
        const std::wstring& raw_name, const std::wstring& local_name);

    /** characters occurs when receiving characters

        @param character
        content of the information received.
    */
    virtual void characters(const std::wstring& character);

protected:
    /** choose an appropriate tag reader to handle the tag.

        @param tag_name
        the name of the tag.
        @param XmlAttributes
        attribute structure of the tag to save in.
    */
    ITag* chooseTagReader(
        const std::wstring& tag_name, const XmlTagAttributes_t& XmlAttributes );

    /** Get the list of style locale pair.

        @return
        the Style-Locale map
    */
    inline StyleLocaleMap_t const & getStyleMap( ) const{ return m_StyleMap; };

    /** get style of the current content.

        @return style of the current content.
    */
    ::std::wstring getCurrentContentStyle( void );

    /** add chunk into Chunk Buffer.
    */
    void addChunk( LocaleSet_t const & Locale, Content_t const & Content );

    /** get a style's locale field.
    */
    LocaleSet_t const & getLocale( const StyleName_t Style );

private:
    std::stack<ITag*> m_TagBuilderStack;

    ChunkBuffer_t   m_ChunkBuffer;
    StyleLocaleMap_t      m_StyleMap;
    LocaleSet_t m_DefaultLocale;
};

#endif