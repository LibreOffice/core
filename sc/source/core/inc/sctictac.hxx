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
#include "address.hxx"
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

