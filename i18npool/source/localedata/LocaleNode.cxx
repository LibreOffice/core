/*************************************************************************
 *
 *  $RCSfile: LocaleNode.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-15 19:09:54 $
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
#include <stdio.h>
#include <stdlib.h>
#include "LocaleNode.hxx"

LocaleNode :: LocaleNode (const OUString& name,
                const Reference< XAttributeList > & attr)
{
    aName = name;
    xAttribs = new Attr(attr);
    nChildren = 0;
    childArrSize = 0;
}

void LocaleNode::print () {
    printf ("<");
    ::rtl::OUString str (aName);
    for(int i = 0; i < str.getLength(); i++)
        printf( "%c", str[i]);
    printf (">\n");
}

void LocaleNode::printR () {
    print();
    for (int i=0;i<nChildren;i++)
        children[i]->printR();
    printf ("\t");
    print();
}

void LocaleNode::addChild ( LocaleNode * node) {
    if (childArrSize <= nChildren) {
        LocaleNode ** arrN = (LocaleNode **)malloc( sizeof (LocaleNode *)*(childArrSize+10) ) ;
        for (int i = 0; i<childArrSize ; i++)
            arrN[i] = children[i];
        if ( childArrSize > 0 )
            free(children);
        childArrSize += 10;
        children = arrN;
    }
    children[nChildren++] = node;
    node->setParent (this);
}

void LocaleNode::setParent ( LocaleNode * node) {
    parent = node;
}

LocaleNode * LocaleNode::findNode ( const OUString & name) {
    if (name == aName)
        return this;
    for (int i = 0; i< nChildren; i++)  {
        LocaleNode *n=children[i]->findNode(name);
        if (n)
            return n;
        }
    return 0;
}

 LocaleNode::~LocaleNode() {
    for (int i=0; i<nChildren;i++)
        delete (children[i]);
}

static
void printf_String( const char* fmt, ::rtl::OUString str )
{
     const int max = 1024;
     char buf[ max+1 ];

     for(int i=0; i<str.getLength() && i<max; i++) buf[i] = str[i];
     buf[i]='\0';
     printf( fmt, buf );
}

LocaleNode* LocaleNode::createNode (const OUString& name,
                                    const Reference< XAttributeList > & attr)
{
    ::rtl::OUString str(name);

    if(str.equals(::rtl::OUString::createFromAscii("LC_INFO")))
        return new LCInfoNode (name,attr);
    if (str.equals(::rtl::OUString::createFromAscii("LC_CTYPE")))
        return new LCCTYPENode (name,attr);
    if (str.equals(::rtl::OUString::createFromAscii("LC_FORMAT")))
        return new LCFormatNode (name,attr);
    if (str.equals(::rtl::OUString::createFromAscii("LC_CALENDAR")))
        return new LCCalendarNode (name,attr);
    if (str.equals(::rtl::OUString::createFromAscii("LC_CURRENCY")))
        return new LCCurrencyNode (name,attr);
    if (str.equals(::rtl::OUString::createFromAscii("LC_TRANSLITERATION")))
        return new LCTransliterationNode (name,attr);
    if (str.equals(::rtl::OUString::createFromAscii("LC_COLLATION")))
        return new LCCollationNode (name,attr);
    if (str.equals(::rtl::OUString::createFromAscii("LC_SEARCH")))
        return new LCSearchNode (name,attr);
    if (str.equals(::rtl::OUString::createFromAscii("LC_MISC")))
        return new LCMiscNode (name,attr);
      if (str.equals(::rtl::OUString::createFromAscii("LC_NumberingLevel")))
                return new LCNumberingLevelNode (name, attr);
      if (str.equals(::rtl::OUString::createFromAscii("LC_OutLineNumberingLevel")))
                return new LCOutlineNumberingLevelNode (name, attr);

    return new LocaleNode(name,attr);

}


//   printf(" name: '%s'\n", p->getName().pData->buffer );
//   printf("value: '%s'\n", p->getValue().pData->buffer );
#include <iostream>

void print_OUString( OUString s )
{
    ::rtl::OUString str (s);
    for(int i=0; i<str.getLength(); i++) printf( "%c", str[i]);
}

bool is_empty( OUString s )
{
     ::rtl::OUString str(s);
     return (str.getLength()==0) || (str.getLength()==1 && str[0]=='\n');
}

void print_indent( int depth )
{
     for( int i=0; i<depth; i++ ) printf("    ");
}

void print_color( int color )
{
     printf("\033[%dm", color);
}

void print_node( LocaleNode* p, int depth=0 )
{
     if( !p ) return;

     print_indent( depth );
     printf("<");
     print_color(36);
     print_OUString( p->getName()  );
     print_color(0);
     const Attr* q = p->getAttr();
     if( q )
     {
          for( int j=0; j<q->getLength(); j++ )
          {
               printf(" ");
               print_color(33);
               print_OUString( q->getTypeByIndex(j) );
               print_color(0);
               printf("=");
               print_color(31);
               printf("'");
               print_OUString( q->getValueByIndex(j) );
               printf("'");
               print_color(0);
          }
     }
     printf(">");
     printf("\n");
     if( !is_empty( p->getValue() ) )
     {
          print_indent( depth+1 );
          printf("value: ");
          print_color(31);
          printf("'");
          print_OUString( p->getValue() );
          printf("'");
          print_color(0);
          printf("\n");
     }
     for( int i=0; i<p->getNumberOfChildren(); i++ )
     {
          print_node( p->getChildAt(i), depth+1 );
     }
     print_indent( depth );
     printf("</");
     print_OUString( p->getName()  );
     printf(">");
     printf("\n");
}

void LocaleNode :: generateCode (const OFileWriter &of) {
    for (int i=0; i<nChildren;i++)
        children[i]->generateCode (of);
//      print_node( this );
}

void LCInfoNode::generateCode (const OFileWriter &of) {

    LocaleNode * languageNode = findNode(::rtl::OUString::createFromAscii("Language"));
    LocaleNode * countryNode = findNode(::rtl::OUString::createFromAscii("Country"));
    LocaleNode * varientNode = findNode(::rtl::OUString::createFromAscii("Varient"));

    if (languageNode) {
        writeParameter(of,"langID", languageNode->getChildAt(0)->getValue());
        writeParameter(of,"langDefaultName", languageNode->getChildAt(1)->getValue());
    }
    if (countryNode) {
        writeParameter(of,"countryID", countryNode->getChildAt(0)->getValue());
        writeParameter(of,"countryDefaultName", countryNode->getChildAt(1)->getValue());
    }
    writeParameter(of,"Varient", ::rtl::OUString::createFromAscii(""));
    of.writeAsciiString("\nstatic const sal_Unicode* LCInfoArray[] = {\n");
    of.writeAsciiString("\tlangID,\n");
    of.writeAsciiString("\tlangDefaultName,\n");
    of.writeAsciiString("\tcountryID,\n");
    of.writeAsciiString("\tcountryDefaultName,\n");
    of.writeAsciiString("\tVarient\n");
    of.writeAsciiString("};\n\n");
}

void LCCTYPENode::generateCode (const OFileWriter &of) {
    ::rtl::OUString str =   getAttr() -> getValueByName(::rtl::OUString::createFromAscii("unoid"));
    LocaleNode * sepNode = findNode(::rtl::OUString::createFromAscii("DateSeparator"));
    of.writeAsciiString("\n\n");
    writeParameter(of,"LC_CTYPE_Unoid", str);;
    writeParameter(of,"dateSeparator", sepNode->getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("ThousandSeparator"));
    writeParameter(of,"thousandSeparator", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("DecimalSeparator"));
    writeParameter(of,"decimalSeparator", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("TimeSeparator"));
    writeParameter(of,"timeSeparator", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("Time100SecSeparator"));
    writeParameter(of,"time100SecSeparator", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("ListSeparator"));
    writeParameter(of,"listSeparator", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("QuotationStart"));
    writeParameter(of,"quotationStart", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("QuotationEnd"));
    writeParameter(of,"quotationEnd",  sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("DoubleQuotationStart"));
    writeParameter(of,"doubleQuotationStart",  sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("DoubleQuotationEnd"));
    writeParameter(of,"doubleQuotationEnd",  sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("TimeAM"));
    writeParameter(of,"timeAM",  sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("TimePM"));
    writeParameter(of,"timePM",  sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("MeasurementSystem"));
    writeParameter(of,"measurementSystem", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("LongDateDayOfWeekSeparator"));
    writeParameter(of,"LongDateDayOfWeekSeparator", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("LongDateDaySeparator"));
    writeParameter(of,"LongDateDaySeparator", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("LongDateMonthSeparator"));
    writeParameter(of,"LongDateMonthSeparator", sepNode -> getValue());
    sepNode = findNode(::rtl::OUString::createFromAscii("LongDateYearSeparator"));
    writeParameter(of,"LongDateYearSeparator", sepNode -> getValue());

    of.writeAsciiString("\nstatic const sal_Unicode* LCType[] = {\n");
    of.writeAsciiString("\tLC_CTYPE_Unoid,\n");
    of.writeAsciiString("\tdateSeparator,\n");
    of.writeAsciiString("\tthousandSeparator,\n");
    of.writeAsciiString("\tdecimalSeparator,\n");
    of.writeAsciiString("\ttimeSeparator,\n");
    of.writeAsciiString("\ttime100SecSeparator,\n");
    of.writeAsciiString("\tlistSeparator,\n");
    of.writeAsciiString("\tquotationStart,\n");
    of.writeAsciiString("\tquotationEnd,\n");
    of.writeAsciiString("\tdoubleQuotationStart,\n");
    of.writeAsciiString("\tdoubleQuotationEnd,\n");
    of.writeAsciiString("\ttimeAM,\n");
    of.writeAsciiString("\ttimePM,\n");
    of.writeAsciiString("\tmeasurementSystem,\n");
    of.writeAsciiString("\tLongDateDayOfWeekSeparator,\n");
    of.writeAsciiString("\tLongDateDaySeparator,\n");
    of.writeAsciiString("\tLongDateMonthSeparator,\n");
    of.writeAsciiString("\tLongDateYearSeparator\n");
    of.writeAsciiString("};\n\n");

}

void LCFormatNode::generateCode (const OFileWriter &of) {
sal_Int32 formatCount = 0;
    for (sal_Int16 i = 0; i< getNumberOfChildren() ; i++,formatCount++) {
        LocaleNode * currNode = getChildAt (i);
            ::rtl::OUString str;
//      currNode -> print();
               const Attr *  currNodeAttr = currNode->getAttr();
            //printf ("getLen() = %d\n", currNode->getAttr()->getLength());
            str = currNodeAttr -> getValueByName(::rtl::OUString::createFromAscii("msgid"));
            writeParameter(of,"FormatKey", str, formatCount);
            str = currNodeAttr -> getValueByName(::rtl::OUString::createFromAscii("default"));
            writeDefaultParameter(of,"FormatElement", str, formatCount);
            str = currNodeAttr -> getValueByName(::rtl::OUString::createFromAscii("type"));
            writeParameter(of,"FormatType", str, formatCount);
            str = currNodeAttr -> getValueByName(::rtl::OUString::createFromAscii("usage"));
            writeParameter(of,"FormatUsage", str, formatCount);
            str = currNodeAttr -> getValueByName(::rtl::OUString::createFromAscii("formatindex"));
            sal_Int16 formatindex = (sal_Int16)str.toInt32();
            writeIntParameter(of,"Formatindex", formatCount, formatindex);

            LocaleNode * n = currNode -> findNode(::rtl::OUString::createFromAscii("FormatCode"));
            if (n)
                    writeParameter(of,"FormatCode", n->getValue(), formatCount);
            n = currNode -> findNode(::rtl::OUString::createFromAscii("DefaultName"));
            if (n)
                writeParameter(of ,"FormatDefaultName", n->getValue(), formatCount);
            else
                writeParameter(of,"FormatDefaultName", ::rtl::OUString::createFromAscii(""), formatCount);

        }
        of.writeAsciiString("\nstatic const sal_Int16 ");
        of.writeAsciiString("FormatElements");
        of.writeAsciiString("Count = ");
        of.writeInt(formatCount);
        of.writeAsciiString(";\n");
        of.writeAsciiString("static const sal_Unicode* ");
        of.writeAsciiString("FormatElements");
        of.writeAsciiString("Array[] = {\n");
        for(i = 0; i < formatCount; i++) {

            of.writeAsciiString("\t");
            of.writeAsciiString("FormatCode");
            of.writeInt(i);
            of.writeAsciiString(",\n");

            of.writeAsciiString("\t");
            of.writeAsciiString("FormatDefaultName");
            of.writeInt(i);
            of.writeAsciiString(",\n");

            of.writeAsciiString("\t");
            of.writeAsciiString("FormatKey");
            of.writeInt(i);
            of.writeAsciiString(",\n");

            of.writeAsciiString("\t");
            of.writeAsciiString("FormatType");
            of.writeInt(i);
            of.writeAsciiString(",\n");

            of.writeAsciiString("\t");
            of.writeAsciiString("FormatUsage");
            of.writeInt(i);
            of.writeAsciiString(",\n");

            of.writeAsciiString("\t");
            of.writeAsciiString("Formatindex");
            of.writeInt(i);
            of.writeAsciiString(",\n");


            of.writeAsciiString("\tdefaultFormatElement");
            of.writeInt(i);
            of.writeAsciiString(",\n");
        }
        of.writeAsciiString("};\n\n");
}

void LCCollationNode::generateCode (const OFileWriter &of) {
    sal_Int16 nbOfCollations = 0;
    sal_Int16 nbOfCollationOptions;
    for (sal_Int16 i = 0; i< getNumberOfChildren();i++) {
        LocaleNode * currNode = getChildAt (i);
        if( currNode->getName().compareToAscii("Collator") == 0 )
        {
            ::rtl::OUString str;
            str = currNode->getAttr() -> getValueByName(::rtl::OUString::createFromAscii("unoid"));
            writeParameter(of,"CollatorID", str, i);
            str = currNode -> getAttr() -> getValueByName(::rtl::OUString::createFromAscii("default"));
            writeDefaultParameter(of,"Collator", str, i);
            of.writeAsciiString("\n");

            nbOfCollations++;
        }
        if( currNode->getName().compareToAscii("CollationOptions") == 0 )
        {
            LocaleNode* pCollationOptions = currNode;
            nbOfCollationOptions = pCollationOptions->getNumberOfChildren();
            for( int i=0; i<nbOfCollationOptions; i++ )
            {
                writeParameter( of, "collationOption", pCollationOptions->getChildAt( i )->getValue(), i );
            }

            of.writeAsciiString("static const sal_Int16 nbOfCollationOptions = ");
            of.writeInt( nbOfCollationOptions );
            of.writeAsciiString(";\n\n");
        }
    }
    of.writeAsciiString("static const sal_Int16 nbOfCollations = ");
    of.writeInt(nbOfCollations);
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("\nstatic const sal_Unicode* LCCollatorArray[] = {\n");
    for(i = 0; i < nbOfCollations; i++) {
        of.writeAsciiString("\tCollatorID");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\tdefaultCollator");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");

    of.writeAsciiString("static const sal_Unicode* collationOptions[] = {");
    for( i=0; i<nbOfCollationOptions; i++ )
    {
        of.writeAsciiString( "collationOption" );
        of.writeInt( i );
        of.writeAsciiString( ", " );
    }
    of.writeAsciiString("NULL };\n");
}

void LCSearchNode::generateCode (const OFileWriter &of)
{
    if( getNumberOfChildren() != 1 )
    {
        exit(1);
    }

    int i;
    LocaleNode* pSearchOptions = getChildAt( 0 );
    int         nSearchOptions = pSearchOptions->getNumberOfChildren();
    for( i=0; i<nSearchOptions; i++ )
    {
        writeParameter( of, "searchOption", pSearchOptions->getChildAt( i )->getValue(), i );
    }

    of.writeAsciiString("static const sal_Int16 nbOfSearchOptions = ");
    of.writeInt( nSearchOptions );
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("static const sal_Unicode* searchOptions[] = {");
    for( i=0; i<nSearchOptions; i++ )
    {
        of.writeAsciiString( "searchOption" );
        of.writeInt( i );
        of.writeAsciiString( ", " );
    }
    of.writeAsciiString("NULL };\n");

}

void LCCalendarNode::generateCode (const OFileWriter &of) {
    sal_Int16 nbOfCalendars = getNumberOfChildren();
    ::rtl::OUString str;
    sal_Int16 * nbOfDays = new sal_Int16[nbOfCalendars];
    sal_Int16 * nbOfMonths = new sal_Int16[nbOfCalendars];
    sal_Int16 * nbOfEras = new sal_Int16[nbOfCalendars];


    for (sal_Int16 i = 0; i < nbOfCalendars; i++) {
        LocaleNode * calNode = getChildAt (i);
        str = calNode -> getAttr() -> getValueByName(::rtl::OUString::createFromAscii("unoid"));
        writeParameter(of, "calendarID", str, i);
        of.writeAsciiString("\n");
        str = calNode -> getAttr() -> getValueByName(::rtl::OUString::createFromAscii("default"));
        writeDefaultParameter(of,"Calendar", str, i);
        LocaleNode * daysNode = calNode -> getChildAt(0);
        nbOfDays[i] = daysNode->getNumberOfChildren();
        // Generate Days of Week
        sal_Char *elementTag = "day";
        for (sal_Int16 j = 0; j < nbOfDays[i]; j++) {
            LocaleNode *currNode = daysNode -> getChildAt(j);
            writeParameter(of,"dayID", currNode->getChildAt(0)->getValue(), i, j);
            of.writeAsciiString("\n");
            writeParameter(of,elementTag, "DefaultAbbrvName",currNode->getChildAt(1)->getValue() ,i, j);
            of.writeAsciiString("\n");
            writeParameter(of,elementTag, "DefaultFullName",currNode->getChildAt(2)->getValue() , i, j);
            of.writeAsciiString("\n");
        }

        LocaleNode * monthsNode = calNode -> getChildAt(1);
        nbOfMonths[i] = monthsNode->getNumberOfChildren();
        // Generate Months of Year
        elementTag = "month";
        for (j = 0; j < nbOfMonths[i]; j++) {
            LocaleNode *currNode = monthsNode -> getChildAt(j);
            writeParameter(of,"monthID", currNode->getChildAt(0)->getValue(), i, j);
            of.writeAsciiString("\n");
            writeParameter(of,elementTag, "DefaultAbbrvName",currNode->getChildAt(1)->getValue() ,i, j);
            of.writeAsciiString("\n");
            writeParameter(of,elementTag, "DefaultFullName",currNode->getChildAt(2)->getValue() , i, j);
            of.writeAsciiString("\n");
        }
        LocaleNode * erasNode = calNode -> getChildAt(2);
        nbOfEras[i] = erasNode->getNumberOfChildren();
        // Generate Months of Year
        elementTag = "era";
        for (j = 0; j < nbOfEras[i]; j++) {
            LocaleNode *currNode = erasNode -> getChildAt(j);
            writeParameter(of,"eraID", currNode->getChildAt(0)->getValue(), i, j);
            of.writeAsciiString("\n");
            writeParameter(of,elementTag, "DefaultAbbrvName",currNode->getChildAt(1)->getValue() ,i, j);
            of.writeAsciiString("\n");
            writeParameter(of,elementTag, "DefaultFullName",currNode->getChildAt(2)->getValue() , i, j);
            of.writeAsciiString("\n");
        }
        writeParameter(of,"startDayOfWeek", calNode ->getChildAt(3)-> getChildAt(0)->getValue(),
                            i);
        of.writeAsciiString("\n");
        str = calNode ->getChildAt(4)-> getValue();
        writeIntParameter(of,"minimalDaysInFirstWeek", i,
                        str . toInt32());
        of.writeAsciiString("\n");
        }
        of.writeAsciiString("static const sal_Int16 calendarsCount = ");
        of.writeInt(nbOfCalendars);
        of.writeAsciiString(";\n\n");

        of.writeAsciiString("static const sal_Unicode nbOfDays[] = {");
        for(i = 0; i < nbOfCalendars - 1; i++) {
            of.writeInt(nbOfDays[i]);
            of.writeAsciiString(", ");
        };
        of.writeInt(nbOfDays[i]);
        of.writeAsciiString("};\n");

        of.writeAsciiString("static const sal_Unicode nbOfMonths[] = {");
        for(i = 0; i < nbOfCalendars - 1; i++) {
            of.writeInt(nbOfMonths[i]);
            of.writeAsciiString(", ");
        };
        of.writeInt(nbOfMonths[i]);
        of.writeAsciiString("};\n");

        of.writeAsciiString("static const sal_Unicode nbOfEras[] = {");
        for(i = 0; i < nbOfCalendars - 1; i++) {
            of.writeInt(nbOfEras[i]);
            of.writeAsciiString(", ");
        };
        of.writeInt(nbOfEras[i]);
        of.writeAsciiString("};\n");


        of.writeAsciiString("static const sal_Unicode* calendars[] = {\n");
        of.writeAsciiString("\tnbOfDays,\n");
        of.writeAsciiString("\tnbOfMonths,\n");
        of.writeAsciiString("\tnbOfEras,\n");
        for(i = 0; i < nbOfCalendars; i++) {
            of.writeAsciiString("\tcalendarID");
            of.writeInt(i);
            of.writeAsciiString(",\n");
            of.writeAsciiString("\tdefaultCalendar");
            of.writeInt(i);
            of.writeAsciiString(",\n");
            for(sal_Int16 j = 0; j < nbOfDays[i]; j++) {
                of.writeAsciiString("\tdayID");
                of.writeInt(i); of.writeInt(j); of.writeAsciiString(",\n");
                of.writeAsciiString("\tdayDefaultAbbrvName");
                of.writeInt(i); of.writeInt(j); of.writeAsciiString(",\n");
                of.writeAsciiString("\tdayDefaultFullName");of.writeInt(i); of.writeInt(j); of.writeAsciiString(",\n");
            }
            for(j = 0; j < nbOfMonths[i]; j++) {
                of.writeAsciiString("\tmonthID");of.writeInt(i);of.writeInt(j);of.writeAsciiString(",\n");
                of.writeAsciiString("\tmonthDefaultAbbrvName");of.writeInt(i);of.writeInt(j);of.writeAsciiString(",\n");
                of.writeAsciiString("\tmonthDefaultFullName");of.writeInt(i);of.writeInt(j);of.writeAsciiString(",\n");
            }
            for(j = 0; j < nbOfEras[i]; j++) {
                of.writeAsciiString("\teraID"); of.writeInt(i); of.writeInt(j); of.writeAsciiString(",\n");
                of.writeAsciiString("\teraDefaultAbbrvName");of.writeInt(i);of.writeInt(j);of.writeAsciiString(",\n");
                of.writeAsciiString("\teraDefaultFullName");of.writeInt(i);of.writeInt(j);of.writeAsciiString(",\n");
            }
            of.writeAsciiString("\tstartDayOfWeek");of.writeInt(i); of.writeAsciiString(",\n");
            of.writeAsciiString("\tminimalDaysInFirstWeek");of.writeInt(i); of.writeAsciiString(",\n");
        }

        of.writeAsciiString("};\n\n");


    delete []nbOfDays;
    delete []nbOfMonths;
    delete []nbOfEras;
}

void LCCurrencyNode :: generateCode (const OFileWriter &of) {
    sal_Int16 nbOfCurrencies = 0;
    ::rtl::OUString str;
    for (sal_Int16 i = 0; i < getNumberOfChildren(); i++,nbOfCurrencies++) {
        LocaleNode * calNode = getChildAt (i);
        str = calNode->getAttr() -> getValueByName(::rtl::OUString::createFromAscii("default"));
        writeDefaultParameter(of,"Currency", str, nbOfCurrencies);
        str = calNode->getAttr() -> getValueByName(::rtl::OUString::createFromAscii("usedInCompatibleFormatCodes"));
        writeDefaultParameter(of,"CurrencyUsedInCompatibleFormatCodes", str, nbOfCurrencies);
        str = calNode -> findNode (::rtl::OUString::createFromAscii("CurrencyID")) -> getValue();
        writeParameter(of,"currencyID", str, nbOfCurrencies);
        str = calNode -> findNode (::rtl::OUString::createFromAscii("CurrencySymbol")) -> getValue();
        writeParameter(of,"currencySymbol", str, nbOfCurrencies);
        str = calNode -> findNode (::rtl::OUString::createFromAscii("BankSymbol")) -> getValue();
        writeParameter(of,"bankSymbol", str, nbOfCurrencies);
        str = calNode -> findNode (::rtl::OUString::createFromAscii("CurrencyName")) -> getValue();
        writeParameter(of,"currencyName", str, nbOfCurrencies);
        str = calNode -> findNode (::rtl::OUString::createFromAscii("DecimalPlaces")) -> getValue();
        sal_Int16 nDecimalPlaces = (sal_Int16)str.toInt32();
        writeIntParameter(of,"currencyDecimalPlaces", nbOfCurrencies, nDecimalPlaces);
        of.writeAsciiString("\n");
    };

    of.writeAsciiString("static const sal_Int16 currencyCount = ");
    of.writeInt(nbOfCurrencies);
    of.writeAsciiString(";\n\n");
    of.writeAsciiString("static const sal_Unicode* currencies[] = {\n");
    for(i = 0; i < nbOfCurrencies; i++) {
        of.writeAsciiString("\tcurrencyID");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tcurrencySymbol");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tbankSymbol");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tcurrencyName");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tdefaultCurrency");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tdefaultCurrencyUsedInCompatibleFormatCodes");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tcurrencyDecimalPlaces");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");

}

void LCTransliterationNode::generateCode (const OFileWriter &of) {
    sal_Int16 nbOfModules = 0;
    ::rtl::OUString str;
    for (sal_Int16 i = 0; i < getNumberOfChildren(); i++,nbOfModules++) {
        LocaleNode * calNode = getChildAt (i);
        str = calNode->getAttr() -> getValueByIndex(0);
        writeParameter(of,"Transliteration", str, nbOfModules);
    }
    of.writeAsciiString("static const sal_Int16 nbOfTransliterations = ");
    of.writeInt(nbOfModules);
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("\nstatic const sal_Unicode* LCTransliterationsArray[] = {\n");
    for( i = 0; i < nbOfModules; i++) {
        of.writeAsciiString("\tTransliteration");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");


}

void LCMiscNode::generateCode (const OFileWriter &of) {
    LocaleNode * reserveNode = findNode(::rtl::OUString::createFromAscii("ReservedWords"));
    LocaleNode * forbidNode = findNode(::rtl::OUString::createFromAscii("ForbiddenCharacters"));

    sal_Int16 nbOfWords = 0;
    ::rtl::OUString str;
    for (sal_Int16 i = 0; i < reserveNode->getNumberOfChildren(); i++,nbOfWords++) {
        LocaleNode * curNode = reserveNode->getChildAt (i);
          str = curNode -> getValue();
          writeParameter(of,"ReservedWord", str, nbOfWords);
    }
    of.writeAsciiString("static const sal_Int16 nbOfReservedWords = ");
    of.writeInt(nbOfWords);
    of.writeAsciiString(";\n\n");
    of.writeAsciiString("\nstatic const sal_Unicode* LCReservedWordsArray[] = {\n");
    for( i = 0; i < nbOfWords; i++) {
        of.writeAsciiString("\tReservedWord");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");

    if (forbidNode)    {
         writeParameter(of, "forbiddenBegin", forbidNode -> getChildAt(0)->getValue());
         writeParameter(of, "forbiddenEnd", forbidNode -> getChildAt(1)->getValue());
    } else {
         writeParameter(of, "forbiddenBegin", ::rtl::OUString::createFromAscii(""));
         writeParameter(of, "forbiddenEnd", ::rtl::OUString::createFromAscii(""));
    }
    of.writeAsciiString("\nstatic const sal_Unicode* LCForbiddenCharactersArray[] = {\n");
    of.writeAsciiString("\tforbiddenBegin,\n");
    of.writeAsciiString("\tforbiddenEnd\n");
    of.writeAsciiString("};\n\n");
}
inline
::rtl::OUString C2U( const char* s )
{
     return ::rtl::OUString::createFromAscii( s );
}

void LCNumberingLevelNode::generateCode (const OFileWriter &of)
{
     of.writeAsciiString("// ---> ContinuousNumbering\n");

     // hard code number of attributes per style.
     const int   nAttributes = 4;
     const char* attr[ nAttributes ] = { "Prefix", "NumType", "Suffix", "Transliteration" };

     // record each attribute of each style in a static C++ variable.
     // determine number of styles on the fly.
     int nStyles = getNumberOfChildren();
     for( int i=0; i<nStyles; i++ )
     {
          const Attr* q = getChildAt( i )->getAttr();
          for( int j=0; j<nAttributes; j++ )
          {
               const char* name = attr[j];
               OUString   value = q->getValueByName( C2U(name) );
               writeParameter( of, "continuous", name, value, i );
          }
     }

     // record number of styles and attributes.
     of.writeAsciiString("static const sal_Int16 continuousNbOfStyles = ");
     of.writeInt( nStyles );
     of.writeAsciiString(";\n\n");
     of.writeAsciiString("static const sal_Int16 continuousNbOfAttributesPerStyle = ");
     of.writeInt( nAttributes );
     of.writeAsciiString(";\n\n");

     // generate code. (intermediate arrays)
     for( i=0; i<nStyles; i++ )
     {
          of.writeAsciiString("\nstatic const sal_Unicode* continuousStyle" );
          of.writeInt( i );
          of.writeAsciiString("[] = {\n");
          for( int j=0; j<nAttributes; j++)
          {
               of.writeAsciiString("\t");
               of.writeAsciiString( "continuous" );
               of.writeAsciiString( attr[j] );
               of.writeInt(i);
               of.writeAsciiString(",\n");
          }
          of.writeAsciiString("\t0\n};\n\n");
     }

     // generate code. (top-level array)
     of.writeAsciiString("\n");
     of.writeAsciiString("static const sal_Unicode** LCContinuousNumberingLevelsArray[] = {\n" );
     for( i=0; i<nStyles; i++ )
     {
          of.writeAsciiString( "\t" );
          of.writeAsciiString( "continuousStyle" );
          of.writeInt( i );
          of.writeAsciiString( ",\n");
     }
     of.writeAsciiString("\t0\n};\n\n");

}

#include <assert.h>

void LCOutlineNumberingLevelNode::generateCode (const OFileWriter &of)
{
     of.writeAsciiString("// ---> OutlineNumbering\n");

     // hardcode number of attributes per level
     const int   nAttributes = 10;
     const char* attr[ nAttributes ] =
     {
          "Prefix",
          "NumType",
          "Suffix",
          "BulletChar",
          "BulletFontName",
          "ParentNumbering",
          "LeftMargin",
          "SymbolTextDistance",
          "FirstLineOffset",
          "Transliteration",
     };

     // record each attribute of each level of each style in a static C++ variable.
     // determine number of styles and number of levels per style on the fly.
     int nStyles = getNumberOfChildren();
     vector<int> nLevels; // may be different for each style?
     for( int i=0; i<nStyles; i++ )
     {
          LocaleNode* p = getChildAt( i );
          nLevels.push_back( p->getNumberOfChildren() );
          for( int j=0; j<nLevels.back(); j++ )
          {
               const Attr* q = p->getChildAt( j )->getAttr();
               for( int k=0; k<nAttributes; k++ )
               {
                    const char* name = attr[k];
                    OUString   value = q->getValueByName( C2U(name) );
                    writeParameter( of, "outline", name, value, i, j );
               }
          }
     }

     // verify that each style has the same number of levels.
     for( i=0; i<nLevels.size(); i++ )
     {
          if( nLevels[0] != nLevels[i] )
          {
               assert(0);
          }
     }

     // record number of attributes, levels, and styles.
     of.writeAsciiString("static const sal_Int16 outlineNbOfStyles = ");
     of.writeInt( nStyles );
     of.writeAsciiString(";\n\n");
     of.writeAsciiString("static const sal_Int16 outlineNbOfLevelsPerStyle = ");
     of.writeInt( nLevels.back() );
     of.writeAsciiString(";\n\n");
     of.writeAsciiString("static const sal_Int16 outlineNbOfAttributesPerLevel = ");
     of.writeInt( nAttributes );
     of.writeAsciiString(";\n\n");

     // too complicated for now...
//     of.writeAsciiString("static const sal_Int16 nbOfOutlineNumberingLevels[] = { ");
//     for( int j=0; j<nStyles; j++ )
//     {
//          of.writeInt( nLevels[j] );
//          of.writeAsciiString(", ");
//     }
//     of.writeAsciiString("};\n\n");


     for( i=0; i<nStyles; i++ )
     {
          for( int j=0; j<nLevels.back(); j++ )
          {
               of.writeAsciiString("static const sal_Unicode* outline");
               of.writeAsciiString("Style");
               of.writeInt( i );
               of.writeAsciiString("Level");
               of.writeInt( j );
               of.writeAsciiString("[] = { ");

               for( int k=0; k<nAttributes; k++ )
               {
                    of.writeAsciiString( "outline" );
                    of.writeAsciiString( attr[k] );
                    of.writeInt( i );
                    of.writeInt( j );
                    of.writeAsciiString(", ");
               }
               of.writeAsciiString("NULL };\n");
          }
     }

     of.writeAsciiString("\n");


     for( i=0; i<nStyles; i++ )
     {
          of.writeAsciiString("static const sal_Unicode** outline");
          of.writeAsciiString( "Style" );
          of.writeInt( i );
          of.writeAsciiString("[] = { ");

          for( int j=0; j<nLevels.back(); j++ )
          {
               of.writeAsciiString("outlineStyle");
               of.writeInt( i );
               of.writeAsciiString("Level");
               of.writeInt( j );
               of.writeAsciiString(", ");
          }
          of.writeAsciiString("NULL };\n");
     }
     of.writeAsciiString("\n");

     of.writeAsciiString("static const sal_Unicode*** LCOutlineNumberingLevelsArray[] = {\n" );
     for( i=0; i<nStyles; i++ )
     {
          of.writeAsciiString( "\t" );
          of.writeAsciiString( "outlineStyle" );
          of.writeInt( i );
          of.writeAsciiString(",\n");
     }
     of.writeAsciiString("\tNULL\n};\n\n");
}

void  LocaleNode::writeIntParameter(const OFileWriter & of,
    const sal_Char* pAsciiStr, const sal_Int16 count, sal_Int16 val) {
    of.writeAsciiString("static const sal_Unicode ");
    of.writeAsciiString(pAsciiStr);
    of.writeInt(count);
    of.writeAsciiString("[] = {");
    of.writeInt(val);
    of.writeAsciiString("};\n");
}

void   LocaleNode::writeDefaultParameter(const OFileWriter & of,const sal_Char* pAsciiStr, const ::rtl::OUString& str, sal_Int16 count) {
    of.writeAsciiString("static const sal_Unicode default");
    of.writeAsciiString(pAsciiStr);
    of.writeInt(count);
    of.writeAsciiString("[] = {");
    if(str.equals(::rtl::OUString::createFromAscii("true")))
        of.writeAsciiString("1};\n");
    else
        of.writeAsciiString("0};\n");
    }

void  LocaleNode::writeDefaultParameter(const OFileWriter & of,const sal_Char* pAsciiStr, const ::rtl::OUString& str) {
        of.writeAsciiString("static const sal_Unicode default");
        of.writeAsciiString(pAsciiStr);
        of.writeAsciiString("[] = {");
        if(str.equals(::rtl::OUString::createFromAscii("true")))
            of.writeAsciiString("1};\n");
        else
            of.writeAsciiString("0};\n");
}

void  LocaleNode::writeParameter(const OFileWriter & of,const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) {
            of.writeAsciiString("static const sal_Unicode ");
            of.writeAsciiString(pAsciiStr);
            of.writeAsciiString("[");
            of.writeAsciiString("] = {");
            of.writeStringCharacters(aChars);
            of.writeAsciiString("0x0};\n");
}

void   LocaleNode::writeParameter(const OFileWriter & of,const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count) {
            of.writeAsciiString("static const sal_Unicode ");
            of.writeAsciiString(pAsciiStr);
            of.writeInt(count);
            of.writeAsciiString("[");
            of.writeAsciiString("] = {");
            of.writeStringCharacters(aChars);
            of.writeAsciiString("0x0};\n");
}

void   LocaleNode::writeParameter(const OFileWriter & of,const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) {
            of.writeAsciiString("static const sal_Unicode ");
            of.writeAsciiString(pAsciiStr);
            of.writeInt(count0);
            of.writeInt(count1);
            of.writeAsciiString("[");
            of.writeAsciiString("] = {");
            of.writeStringCharacters(aChars);
            of.writeAsciiString("0x0};\n");
}

void   LocaleNode::writeParameter(const OFileWriter & of,const sal_Char* pTagStr, const sal_Char* pAsciiStr,
                                          const ::rtl::OUString& aChars, const sal_Int16 count) {
            of.writeAsciiString("static const sal_Unicode ");
            of.writeAsciiString(pTagStr);
            of.writeAsciiString(pAsciiStr);
            of.writeInt(count);
            of.writeAsciiString("[");
            of.writeAsciiString("] = {");
            of.writeStringCharacters(aChars);
            of.writeAsciiString("0x0};\n");
}

void   LocaleNode::writeParameter(const OFileWriter & of,const sal_Char* pTagStr,
                                         const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) {
            of.writeAsciiString("static const sal_Unicode ");
            of.writeAsciiString(pTagStr);
            of.writeAsciiString(pAsciiStr);
            of.writeAsciiString("[");
            of.writeAsciiString("] = {");
            of.writeStringCharacters(aChars);
            of.writeAsciiString("0x0};\n");
}

void   LocaleNode::writeParameter(const OFileWriter & of,const sal_Char* pTagStr,
                                          const sal_Char* pAsciiStr, const ::rtl::OUString& aChars,
                                          sal_Int16 count0, sal_Int16 count1) {
            of.writeAsciiString("static const sal_Unicode ");
            of.writeAsciiString(pTagStr);
            of.writeAsciiString(pAsciiStr);
            of.writeInt(count0);
            of.writeInt(count1);
            of.writeAsciiString("[");
            of.writeAsciiString("] = {");
            of.writeStringCharacters(aChars);
            of.writeAsciiString("0x0};\n");
}

Attr::Attr (const Reference< XAttributeList > & attr) {
    int len = attr->getLength();
    name.realloc (len);
    value.realloc (len);
    for (int i =0; i< len;i++) {
        name[i] = attr->getNameByIndex(i);
        value[i] = attr -> getValueByIndex(i);
    }
}

OUString Attr::getValueByName (const OUString & str) const {
    int len = name.getLength();
    OUString result;
    for (int i = 0;i<len;i++) {
        if (name[i] == str) {
            return value[i];
            }
            }
    return result;
}
sal_Int32 Attr::getLength() const{
    return name.getLength();
}

OUString Attr::getTypeByIndex (sal_Int32 idx) const {
    return name[idx];
}

OUString Attr::getValueByIndex (sal_Int32 idx) const
{
    return value[idx];
}
