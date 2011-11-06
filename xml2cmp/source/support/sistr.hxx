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



#ifndef XML2CMP_SISTR_HXX
#define XML2CMP_SISTR_HXX


class  Simstr
{
// INTERFACE
   public:
      // Constructors, destructor, '=' and typecasts
                     Simstr(
                           const char *   str = 0);
                     Simstr(                       // Creates Simstr out of a copy of the described bytes within 'anyBytes'.
                                                   // Adds a '\0' at the end.
                           const char *   anybytes,
                           int            firstBytesPos,
                           int            nrOfBytes);
      virtual        ~Simstr();
                     Simstr(
                           const Simstr & S);
      Simstr &       operator=(
                           const Simstr & S);
                     operator const char*() const;

      // diverse utility functions
      const char *   str() const                { return sz; }
      char *         s();   // ATTENTION !!!       // Only to be used, when a function needs a 'char*' but
                                                   //   nevertheless THAT WILL BE NOT CHANGED!
                                                   //   Typecasts to 'const char*' are performed automatically.
      int            l() const;                    // Length of string without '\0' at end.
      Simstr         operator+(
                           const Simstr & S) const;
      Simstr &       operator+=(
                           const Simstr & S);

      // comparison operators
      bool           operator==(
                           const Simstr & S) const;
      bool           operator!=(
                           const Simstr & S) const;
      bool           operator<(
                           const Simstr & S) const;
      bool           operator>(
                           const Simstr & S) const;
      bool           operator<=(
                           const Simstr & S) const;
      bool           operator>=(
                           const Simstr & S) const;


      // 'List of characters' - functions
         // insert - functions
      void           push_front(
                           char           c);
      void           push_back(
                           char           c);
      void           push_front(
                           const Simstr & S);
      void           push_back(
                           const Simstr & S);
         // remove - functions
      void           remove(
                             int            pos,
                           int            anzahl = 1);
      void           remove_trailing_blanks();

         // search functions
      int            pos_first(
                           char           c) const;
      int            pos_last(
                           char           c) const;
      bool           is_empty() const;                         // Only true if object == "".
      bool           is_no_text() const;                       // String may contain spaces or tabs.

         // substitution functions
      void           replace_all(
                           char           oldCh,
                           char           newCh);
         // token functions
            // get...-functions return the token, separated by char 'c' and leave the object unchanged.
            // take...-functions return the same, but remove the token and the corresponding separator from the object.
      Simstr         get_last_token(
                           char           c) const;

   private:
      char *         sz;
      int            len;
};

// Simstr - char* / char - concatenations
Simstr operator+(const char * str, const Simstr & S);
Simstr operator+(const Simstr & S, const char * str);
Simstr operator+(char c, const Simstr & S);
Simstr operator+(const Simstr & S, char c);

// Simstr - char* - comparison operators
bool  operator==(const Simstr & S, const char * str);
bool  operator!=(const Simstr & S, const char * str);
bool  operator<(const Simstr & S, const char * str);
bool  operator>(const Simstr & S, const char * str);
bool  operator<=(const Simstr & S, const char * str);
bool  operator>=(const Simstr & S, const char * str);
bool  operator==(const char * str, const Simstr & S);
bool  operator!=(const char * str, const Simstr & S);
bool  operator<(const char * str, const Simstr & S);
bool  operator>(const char * str, const Simstr & S);
bool  operator<=(const char * str, const Simstr & S);
bool  operator>=(const char * str, const Simstr & S);


inline char *
Simstr::s()                            { return sz; }
inline int
Simstr::l() const                      { return len; }
inline
Simstr::operator const char*() const   { return sz; }
inline bool
Simstr::is_empty() const               { return len == 0; }


#endif

