/*************************************************************************
 *
 *  $RCSfile: sctictac.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:16 $
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

#ifndef SC_SCTICTAC_HXX
#define SC_SCTICTAC_HXX

//#define TICTACTOE_MAIN
#ifdef TICTACTOE_MAIN
#define TICTACTOE_STDOUT
#else
//#define TICTACTOE_STDOUT
#endif
#ifndef TICTACTOE_STDOUT
#define TICTACTOE_SC
#endif

#ifdef TICTACTOE_SC
class ScDocument;
#include "global.hxx"
#else
#include <tools/string.hxx>
#endif

static const int ScTicTacToe_Squares = 9;
static const int ScTicTacToe_Possible_Wins = 8;
typedef sal_Unicode Square_Type;
typedef Square_Type Board_Type[ScTicTacToe_Squares];

class ScTicTacToe
{
private:
    /* Structure to hold a move and its heuristic */
    typedef struct {
        int Square;
        int Heuristic;
    } Move_Heuristic_Type;

    static  const Square_Type   Empty;
    static  const Square_Type   Human;
    static  const Square_Type   Compi;
    static  const int       Infinity;       /* Higher value than any score */
    static  const int       Maximum_Moves;  /* Maximum moves in a game */

            Board_Type      Board;
#ifdef TICTACTOE_SC
            ScAddress       aPos;           // linke obere Ecke des Boards
            ScDocument*     pDoc;
#endif
            ByteString      aStdOut;
            int             Total_Nodes;    /* Nodes searched with minimax */
            int             nMove;
            Square_Type     aPlayer;
            BOOL            bInitialized;

    /* Array describing the eight combinations of three squares in a row */
    static  const int       Three_in_a_Row[ScTicTacToe_Possible_Wins][3];

    /* Array used in heuristic formula for each move. */
    static  const int       Heuristic_Array[4][4];


            Square_Type     Winner();
    inline  Square_Type     Other( Square_Type Player );
    inline  void            Play( int Square, Square_Type Player );
            int             Evaluate( Square_Type Player );
            int             BestMove( Square_Type Player, int *Square,
                                    int Move_Nbr, int Alpha, int Beta );
            void            Describe( int Score );
            void            Move( int& Square );
            Square_Type     TryMove( int& Square );     // return Winner()
            void            PromptHuman();
#ifdef TICTACTOE_SC
            // -1 == Fehler/Redraw, 0 == keine Aenderung, >0 == UserMoveSquare+1
            int             GetStatus();
            void            DrawBoard();
            void            DrawPos( int nSquare, const String& rStr );
#endif
#ifdef TICTACTOE_STDOUT
            void            Print();
#endif

                            ScTicTacToe( const ScTicTacToe& );
            ScTicTacToe&    operator=( const ScTicTacToe& );

public:
#ifdef TICTACTOE_SC
                            ScTicTacToe( ScDocument* pDoc, const ScAddress& );
#else
                            ScTicTacToe();
#endif
                            ~ScTicTacToe() {}
            void            Initialize( BOOL bHumanFirst );
            Square_Type     GetEmpty() { return Empty; }
#ifdef TICTACTOE_SC
            Square_Type     CalcMove();     // return Winner()
#endif
#ifdef TICTACTOE_STDOUT
            void            Game();
            void            GetOutput( ByteString& rStr );
#else
            void            GetOutput( String& rStr );
#endif
};

#endif

