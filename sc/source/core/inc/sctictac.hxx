/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sctictac.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:35:45 $
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

