/*************************************************************************
 *
 *  $RCSfile: FStringFunctions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:28:25 $
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

#ifndef _CONNECTIVITY_FILE_FSTRINGFUNCTIONS_HXX_
#define _CONNECTIVITY_FILE_FSTRINGFUNCTIONS_HXX_

#include "file/fcode.hxx"


namespace connectivity
{
    class OSQLParseNode;
    namespace file
    {
        /** UCASE(str)
            UPPER(str)
                Returns the string str with all characters changed to uppercase according to the current character set mapping (the default is ISO-8859-1 Latin1):

            > SELECT UCASE('Hej');
                    -> 'HEJ'

        */
        class OOp_Upper : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const;
        };

        /** LCASE(str)
            LOWER(str)
                Returns the string str with all characters changed to lowercase according to the current character set mapping (the default is ISO-8859-1 Latin1):

            > SELECT LCASE('QUADRATICALLY');
                    -> 'quadratically'

        */
        class OOp_Lower : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const;
        };

        /** ASCII(str)
            Returns the ASCII code value of the leftmost character of the string str. Returns 0 if str is the empty string. Returns NULL if str is NULL:

            > SELECT ASCII('2');
                -> 50
            > SELECT ASCII(2);
                -> 50
            > SELECT ASCII('dx');
                -> 100

        */
        class OOp_Ascii : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const;
        };

        /** LENGTH(str)
            OCTET_LENGTH(str)
            CHAR_LENGTH(str)
            CHARACTER_LENGTH(str)
                Returns the length of the string str:

            > SELECT LENGTH('text');
                    -> 4
            > SELECT OCTET_LENGTH('text');
                    -> 4

        */
        class OOp_CharLength : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const;
        };

        /** CHAR(N,...)
            CHAR() interprets the arguments as integers and returns a string consisting of the characters given by the ASCII code values of those integers. NULL values are skipped:

            > SELECT CHAR(ascii('t'),ascii('e'),ascii('s'),ascii('t'));
                -> 'test'
            > SELECT CHAR(77,77.3,'77.3');
                -> 'MMM'

        */
        class OOp_Char : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const;
        };

        /** CONCAT(str1,str2,...)
            Returns the string that results from concatenating the arguments. Returns NULL if any argument is NULL. May have more than 2 arguments. A numeric argument is converted to the equivalent string form:

            > SELECT CONCAT('OO', 'o', 'OO');
                -> 'OOoOO'
            > SELECT CONCAT('OO', NULL, 'OO');
                -> NULL
            > SELECT CONCAT(14.3);
                -> '14.3'

        */
        class OOp_Concat : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const;
        };

        /** LOCATE(substr,str)
            POSITION(substr IN str)
            Returns the position of the first occurrence of substring substr in string str. Returns 0 if substr is not in str:

            > SELECT LOCATE('bar', 'foobarbar');
                    -> 4
            > SELECT LOCATE('xbar', 'foobar');
                    -> 0
            LOCATE(substr,str,pos)
            Returns the position of the first occurrence of substring substr in string str, starting at position pos. Returns 0 if substr is not in str:

            > SELECT LOCATE('bar', 'foobarbar',5);
                -> 7

        */
        class OOp_Locate : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const;
        };

        /** SUBSTRING(str,pos)
            SUBSTRING(str FROM pos)
                Returns a substring from string str starting at position pos:

            > SELECT SUBSTRING('Quadratically',5);
                    -> 'ratically'
            > SELECT SUBSTRING('foobarbar' FROM 4);
                    -> 'barbar'
            SUBSTRING(str,pos,len)
            SUBSTRING(str FROM pos FOR len)
                Returns a substring len characters long from string str, starting at position pos. The variant form that uses FROM is SQL-92 syntax:

            > SELECT SUBSTRING('Quadratically',5,6);
                    -> 'ratica'

        */
        class OOp_SubString : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const;
        };

        /** LTRIM(str)
            Returns the string str with leading space characters removed:

            > SELECT LTRIM('  barbar');
                -> 'barbar'

        */
        class OOp_LTrim : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const;
        };

        /** RTRIM(str)
            Returns the string str with trailing space characters removed:

            > SELECT RTRIM('barbar   ');
                -> 'barbar'

        */
        class OOp_RTrim : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const;
        };

        /** SPACE(N)
            Returns a string consisting of N space characters:

            > SELECT SPACE(6);
                -> '      '

        */
        class OOp_Space : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const;
        };

        /** REPLACE(str,from_str,to_str)
            Returns the string str with all occurrences of the string from_str replaced by the string to_str:

            > SELECT REPLACE('www.OOo.com', 'w', 'Ww');
                -> 'WwWwWw.OOo.com'

        */
        class OOp_Replace : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const;
        };

        /** REPEAT(str,count)
            Returns a string consisting of the string str repeated count times. If count <= 0, returns an empty string. Returns NULL if str or count are NULL:

            > SELECT REPEAT('OOo', 3);
                -> 'OOoOOoOOo'

        */
        class OOp_Repeat : public OBinaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const;
        };

        /** INSERT(str,pos,len,newstr)
            Returns the string str, with the substring beginning at position pos and len characters long replaced by the string newstr:

            > SELECT INSERT('Quadratic', 3, 4, 'What');
                -> 'QuWhattic'

        */
        class OOp_Insert : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const;
        };

        /** LEFT(str,len)
            Returns the leftmost len characters from the string str:

            > SELECT LEFT('foobarbar', 5);
                -> 'fooba'

        */
        class OOp_Left : public OBinaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const;
        };

        /** RIGHT(str,len)
            Returns the rightmost len characters from the string str:

            > SELECT RIGHT('foobarbar', 4);
                -> 'rbar'
        */
        class OOp_Right : public OBinaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const;
        };
    }
}

#endif // _CONNECTIVITY_FILE_FCODE_HXX_

