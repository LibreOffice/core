/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: segmentedname.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:14:54 $
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
#include <ary/segmentedname.hxx>


// NOT FULLY DEFINED SERVICES



namespace ary
{
namespace sn    // segmented name
{


//*********************     NameSegment     ******************//

NameSegment::NameSegment( const char * i_name )
    :   sName(i_name),
        pTemplateArguments()
{
}

NameSegment::NameSegment( const String & i_name )
    :   sName(i_name),
        pTemplateArguments()
{
}

NameSegment::NameSegment( const NameSegment & i_other )
    :   sName(i_other.sName),
        pTemplateArguments()
{
    if (i_other.TemplateArguments() != 0)
        pTemplateArguments = new String(*i_other.TemplateArguments());
}

NameSegment::~NameSegment()
{
}

NameSegment &
NameSegment::operator=( const NameSegment & i_other )
{
    sName = i_other.sName;
    if (i_other.TemplateArguments() != 0)
        pTemplateArguments = new String(*i_other.TemplateArguments());
    return *this;
}

bool
NameSegment::operator<( const NameSegment & i_other ) const
{
    if (sName < i_other.sName)
        return true;
    else if (sName == i_other.sName)
    {
        if (i_other.TemplateArguments() == 0)
            return false;
        else if (TemplateArguments() == 0)
            return true;
        else
            return *TemplateArguments() < *i_other.TemplateArguments();
    }

    return false;
}

String &
NameSegment::Make_Template()
{
    return *(pTemplateArguments = new String);
}

void
NameSegment::Get_Text( StreamStr &  o_out ) const
{
    o_out << sName;
    if ( pTemplateArguments )
        o_out << '<' << *pTemplateArguments << '>';
}


}   // namespace sn





//*********************     SegmentedName      ******************//
SegmentedName::SegmentedName( const char * i_text )
    :   aSegments(),
        bIsAbsolute(strncmp(i_text,"::",2) == 0)
{
    Interpret_Text(i_text + (bIsAbsolute ? 2 : 0));
}

SegmentedName::SegmentedName( const String &      i_text )
    :   aSegments(),
        bIsAbsolute(strncmp(i_text.c_str(),"::",2) == 0)
{
    Interpret_Text(i_text.c_str() + (bIsAbsolute ? 2 : 0));
}

SegmentedName::~SegmentedName()
{
}

bool
SegmentedName::operator<( const SegmentedName & i_other ) const
{
    if (IsAbsolute() AND NOT i_other.IsAbsolute())
        return true;
    if (NOT IsAbsolute() AND i_other.IsAbsolute())
        return false;
    return aSegments < i_other.aSegments;
}

void
SegmentedName::Get_Text( StreamStr & o_out ) const
{
    if ( IsAbsolute() )
        o_out << "::";

    sn::NameChain::const_iterator
        it = aSegments.begin();
    sn::NameChain::const_iterator
        itEnd = aSegments.end();
    if (it != itEnd)
        (*it).Get_Text(o_out);
    for ( ++it;
          it != itEnd;
          ++it )
    {
        o_out << "::";
        (*it).Get_Text(o_out);
    }
    o_out << '>';
}


void
SegmentedName::Interpret_Text( const char * i_text )
{
    const int blank = 1;
    const int name = 2;
    const int delimiter = 3;
    const int open_tpl = 4;

    const int
        C_aTextInterpreter[128] =
    {
        0, 0, 0, 0, 0, 0, 0, 0,
            0, blank, blank, 0, 0, blank, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
        blank, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
        name, name, name, name, name, name, name, name,
            name, name, delimiter, 0, open_tpl, 0, 0, 0,

        0, name, name, name, name, name, name, name,
            name, name, name, name, name, name, name, name,
        name, name, name, name, name, name, name, name,
            name, name, name, 0, 0, 0, 0, name,
        0, name, name, name, name, name, name, name,
            name, name, name, name, name, name, name, name,
        name, name, name, name, name, name, name, name,
            name, name, name, 0, 0, 0, 0, 0
    };

    Dyn<sn::NameSegment>
                        pNextSegment(0);
    const char *        pTokenStart = i_text;

    intt                nDelimiterStep = 0;    // 0 = in-name,
                                               // 1 = first  ':' found
    uintt               nTemplateLevel = 0;
    bool                bTokenFinished = false;

    const char *        iter = i_text;
    for ( ; *iter != 0; ++iter)
    {
        if (nTemplateLevel == 0)
        {
            switch (C_aTextInterpreter[int(*iter)])
            {
                case name:
                        if (bTokenFinished)
                        {
                            aSegments.push_back(*pNextSegment.Release());
                            pTokenStart = iter;
                            nDelimiterStep = 0;
                            bTokenFinished = false;
                        }
                        break;
                case delimiter:
                        if (nDelimiterStep == 0)
                        {
                            pNextSegment = new sn::NameSegment(String(pTokenStart,iter));
                            bTokenFinished = true;
                        }
                        ++nDelimiterStep;
                        break;
                case open_tpl:
                        pNextSegment = new sn::NameSegment(String(pTokenStart,iter));
                        bTokenFinished = true;
                        break;
                default:
                    csv_assert(false);
            }   // end switch
        }
        else    // nTemplateLevel > 0
        {
            if (bTokenFinished)
            {
                pTokenStart = iter;
                bTokenFinished = false;
            }
            if (*iter == '<')
                ++nTemplateLevel;
            else if (*iter == '>')
            {
                --nTemplateLevel;
                if (nTemplateLevel == 0)
                {
                    csv_assert(pNextSegment);
                    pNextSegment->Make_Template() = String(pTokenStart, iter);
                    bTokenFinished = true;
                }
            }
        }   // end if (nTemplateLevel == 0)
    }   // end for

    if (NOT bTokenFinished AND pTokenStart != iter)
    {
        pNextSegment = new sn::NameSegment(String(pTokenStart,iter));
        aSegments.push_back(*pNextSegment.Release());
    }
}


}   // namespace ary
