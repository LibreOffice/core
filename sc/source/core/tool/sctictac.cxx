/*************************************************************************
 *
 *  $RCSfile: sctictac.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:18 $
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

/* Tic-Tac-Toe program by Steve Chapel schapel@cs.ucsb.edu
   Uses alpha-beta pruning minimax search to play a "perfect" game.
   The alpha-beta pruning can be removed, but will increase search time.
   The heuristic and move ordering in BestMove() can also be removed with
     an increase in search time. */

#include <stdio.h>
#include <ctype.h>

#pragma hdrstop

#include "sctictac.hxx"

#ifdef TICTACTOE_SC
#include "document.hxx"
#include "cell.hxx"
#endif

const Square_Type ScTicTacToe::Empty = ' ';
const Square_Type ScTicTacToe::Human = 'X';
const Square_Type ScTicTacToe::Compi = 'O';
const int ScTicTacToe::Infinity = 10;       /* Higher value than any score */
const int ScTicTacToe::Maximum_Moves = ScTicTacToe_Squares; /* Maximum moves in a game */

/* Array describing the eight combinations of three squares in a row */
const int ScTicTacToe::Three_in_a_Row[ScTicTacToe_Possible_Wins][3] = {
    { 0, 1, 2 },
    { 3, 4, 5 },
    { 6, 7, 8 },
    { 0, 3, 6 },
    { 1, 4, 7 },
    { 2, 5, 8 },
    { 0, 4, 8 },
    { 2, 4, 6 }
};

/* Array used in heuristic formula for each move. */
const int ScTicTacToe::Heuristic_Array[4][4] = {
    {     0,   -10,  -100, -1000 },
    {    10,     0,     0,     0 },
    {   100,     0,     0,     0 },
    {  1000,     0,     0,     0 }
};


#ifdef TICTACTOE_SC
ScTicTacToe::ScTicTacToe( ScDocument* pDocP, const ScAddress& rPos ) :
        pDoc( pDocP ),
        aPos( rPos ),
        bInitialized( FALSE ),
        aStdOut( "Computer plays O, you play X. " )
{
}
#else
ScTicTacToe::ScTicTacToe() :
        bInitialized( FALSE ),
        aStdOut( "Computer plays O, you play X. " )
{
}
#endif


/* Return the other player */
inline Square_Type ScTicTacToe::Other(Square_Type Player)
{
    return Player == Human ? Compi : Human;
}


/* Make a move on the board */
inline void ScTicTacToe::Play(int Square, Square_Type Player)
{
    Board[Square] = Player;
}


#ifdef TICTACTOE_STDOUT

void ScTicTacToe::GetOutput( ByteString& rStr )
{
     rStr = aStdOut;
     aStdOut.Erase();
}

#else // !TICTACTOE_STDOUT

void ScTicTacToe::GetOutput( String& rStr )
{
     rStr = String( aStdOut, gsl_getSystemTextEncoding() );
     aStdOut.Erase();
}

#endif // TICTACTOE_STDOUT


/* Clear the board */
void ScTicTacToe::Initialize( BOOL bHumanFirst )
{
    bInitialized = TRUE;
    aPlayer = (bHumanFirst ? Human : Compi);
    nMove = 1;
    for (int i = 0; i < ScTicTacToe_Squares; i++)
        Board[i] = Empty;
}


/* If a player has won, return the winner. If the game is a tie,
   return 'C' (for cat). If the game is not over, return Empty. */
Square_Type ScTicTacToe::Winner()
{
    int i;
    for (i = 0; i < ScTicTacToe_Possible_Wins; i++)
    {
        Square_Type Possible_Winner = Board[Three_in_a_Row[i][0]];
        if (Possible_Winner != Empty &&
            Possible_Winner == Board[Three_in_a_Row[i][1]] &&
            Possible_Winner == Board[Three_in_a_Row[i][2]])
            return Possible_Winner;
    }

    for (i = 0; i < ScTicTacToe_Squares; i++)
    {
        if (Board[i] == Empty)
            return Empty;
    }

    return 'C';
}


/* Return a heuristic used to determine the order in which the
   children of a node are searched */
int ScTicTacToe::Evaluate(Square_Type Player)
{
    int i;
    int Heuristic = 0;
    for (i = 0; i < ScTicTacToe_Possible_Wins; i++)
    {
        int j;
        int Players = 0, Others = 0;
        for (j = 0; j < 3; j++)
        {
            Square_Type Piece = Board[Three_in_a_Row[i][j]];
            if (Piece == Player)
                Players++;
            else if (Piece == Other(Player))
                Others++;
        }
        Heuristic += Heuristic_Array[Players][Others];
    }
    return Heuristic;
}


/* Return the score of the best move found for a board
   The square to move to is returned in *Square */
int ScTicTacToe::BestMove(Square_Type Player, int *Square,
        int Move_Nbr, int Alpha, int Beta)
{
    int Best_Square = -1;
    int Moves = 0;
    int i;
    Move_Heuristic_Type Move_Heuristic[ScTicTacToe_Squares];

    Total_Nodes++;

    /* Find the heuristic for each move and sort moves in descending order */
    for (i = 0; i < ScTicTacToe_Squares; i++)
    {
        if (Board[i] == Empty)
        {
            int Heuristic;
            int j;
            Play(i, Player);
            Heuristic = Evaluate(Player);
            Play(i, Empty);
            for (j = Moves-1; j >= 0 &&
                    Move_Heuristic[j].Heuristic < Heuristic; j--)
            {
                Move_Heuristic[j + 1].Heuristic = Move_Heuristic[j].Heuristic;
                Move_Heuristic[j + 1].Square = Move_Heuristic[j].Square;
            }
            Move_Heuristic[j + 1].Heuristic = Heuristic;
            Move_Heuristic[j + 1].Square = i;
            Moves++;
        }
    }

    for (i = 0; i < Moves; i++)
    {
        int Score;
        int Sq = Move_Heuristic[i].Square;
        Square_Type W;

        /* Make a move and get its score */
        Play(Sq, Player);

        W = Winner();
        if (W == Compi)
            Score = (Maximum_Moves + 1) - Move_Nbr;
        else if (W == Human)
            Score = Move_Nbr - (Maximum_Moves + 1);
        else if (W == 'C')
            Score = 0;
        else
            Score = BestMove(Other(Player), Square, Move_Nbr + 1,
                    Alpha, Beta);

        Play(Sq, Empty);

        /* Perform alpha-beta pruning */
        if (Player == Compi)
        {
            if (Score >= Beta)
            {
                *Square = Sq;
                return Score;
            }
            else if (Score > Alpha)
            {
                Alpha = Score;
                Best_Square = Sq;
            }
        }
        else
        {
            if (Score <= Alpha)
            {
                *Square = Sq;
                return Score;
            }
            else if (Score < Beta)
            {
                Beta = Score;
                Best_Square = Sq;
            }
        }
    }
    *Square = Best_Square;
    if (Player == Compi)
        return Alpha;
    else
        return Beta;
}


/* Provide an English description of the score returned by BestMove */
void ScTicTacToe::Describe(int Score)
{
    if (Score < 0)
        aStdOut += "You have a guaranteed win. ";
    else if (Score == 0)
        aStdOut += "I can guarantee a tie. ";
    else
    {
        aStdOut += "I have a guaranteed win by move ";
        aStdOut += ByteString::CreateFromInt32( Maximum_Moves - Score + 1 );
        aStdOut += ". ";
    }
}


/* Have the human or the computer move */
void ScTicTacToe::Move( int& Square )
{
    if (aPlayer == Compi)
    {
        Total_Nodes = 0;
        Describe(BestMove(aPlayer, &Square, nMove, -Infinity, Infinity));
        aStdOut += ByteString::CreateFromInt32( Total_Nodes );
        aStdOut += " nodes examined. ";
        Play(Square, aPlayer);
        aStdOut += "Move #";
        aStdOut += ByteString::CreateFromInt32( nMove );
        aStdOut += " - O moves to ";
        aStdOut += ByteString::CreateFromInt32( Square + 1 );
        aStdOut += ". ";
        aPlayer = Other( aPlayer );
        nMove++;
    }
    else
    {
        if ( Square < 0 || Square >= ScTicTacToe_Squares
                || Board[Square] != Empty )
            Square = -1;
        else
        {
            Play(Square, aPlayer);
            aPlayer = Other( aPlayer );
            nMove++;
        }
    }
}


// Try a move
Square_Type ScTicTacToe::TryMove( int& Square )
{
    if ( !bInitialized )
        Initialize( FALSE );

    Square_Type W = Winner();
    if ( W == Empty )
    {
        Move( Square );
#ifdef TICTACTOE_STDOUT
        if ( aStdOut.Len() )
        {
            puts( aStdOut.GetBuffer() );
            aStdOut.Erase();
        }
#endif
        W = Winner();
    }
    if ( W == Empty )
    {
        if ( aPlayer == Human )
            PromptHuman();
    }
    else
    {
        if (W != 'C')
        {
            aStdOut += W;
            aStdOut += " wins!";
        }
        else
            aStdOut += "It's a tie.";
    }
    return W;
}


void ScTicTacToe::PromptHuman()
{
    aStdOut += "Move #";
    aStdOut += ByteString::CreateFromInt32( nMove );
    aStdOut += " - What is X's move?";
}


#ifdef TICTACTOE_SC

void ScTicTacToe::DrawPos( int nSquare, const String& rStr )
{
    pDoc->SetString( aPos.Col()+(nSquare%3),
        aPos.Row()+(nSquare/3), aPos.Tab(), rStr );
}


void ScTicTacToe::DrawBoard()
{
    String aStr;
    for ( USHORT j = 0; j < ScTicTacToe_Squares; j++ )
    {
        aStr = Board[j];
        DrawPos( j, aStr );
    }
}


// -1 == Fehler/Redraw, 0 == keine Aenderung, >0 == UserMoveSquare+1
int ScTicTacToe::GetStatus()
{
    USHORT nCol, nRow, nTab;
    nCol = aPos.Col();
    nRow = aPos.Row();
    nTab = aPos.Tab();
    String aStr;
    int nDiffs = 0;
    int nSquare;
    for ( USHORT j = 0; j < ScTicTacToe_Squares; j++ )
    {
        pDoc->GetString( nCol+(j%3), nRow+(j/3), nTab, aStr );
        if ( !aStr.Len() )
        {
            if ( Board[j] != Empty )
                return -1;          // wo was sein muss muss was sein
        }
        else
        {
            aStr.ToUpperAscii();
            if ( aStr.GetChar(0) != Board[j] )
            {
                if ( Board[j] != Empty )
                    return -1;      // bestehendes ueberschrieben
                                    // bei erstem Move hat Human angefangen
                if ( ++nDiffs > 1 )
                    return -1;      // mehr als eine Aenderung
                nSquare = j;
            }
        }
    }
    if ( nDiffs == 1 )
        return nSquare + 1;
    return 0;
}


Square_Type ScTicTacToe::CalcMove()
{
    Square_Type W = Winner();
    int nStat = GetStatus();
    if ( nStat || (W == Empty && aPlayer == Compi) )
    {
        if ( nStat == -1 || (nStat > 0 && aPlayer == Compi) )
            DrawBoard();
        if ( W == Empty && aPlayer == Human )
        {
            if ( nStat > 0 )
            {
                int nSquare = --nStat;
                W = TryMove( nStat );
                if ( nStat == -1 )
                    DrawPos( nSquare, String( ' ' ) );
                else
                    DrawPos( nStat, String( Human ) );
            }
            else
                PromptHuman();
        }
        if ( W == Empty && aPlayer == Compi )
        {
            W = TryMove( nStat );       // ComputerMove, nStat egal
            DrawPos( nStat, String( Compi ) );
        }
    }
    else if ( W == Empty && aPlayer == Human )
        PromptHuman();
    return W;
}

#endif // TICTACTOE_SC


#ifdef TICTACTOE_STDOUT
/* Print the board */
void ScTicTacToe::Print()
{
    int i;
    for (i = 0; i < ScTicTacToe_Squares; i += 3)
    {
        if (i > 0)
            printf("---+---+---\n");
        printf(" %c | %c | %c \n", Board[i], Board[i + 1], Board[i + 2]);
    }
    printf("\n");
}


/* Play a game of tic-tac-toe */
void ScTicTacToe::Game()
{
    if ( !bInitialized )
        Initialize( FALSE );

    int Square = (aPlayer == Compi ? 0 : -1);
    Square_Type W = Winner();
    while( W == Empty )
    {
        Print();
        W = TryMove( Square );
        if ( W == Empty )
        {
            if ( aPlayer == Human )
            {
                if ( Square != -1 )
                    Print();    // empty board already printed if human moves first
                do
                {
                    puts( aStdOut.GetBuffer() );
                    aStdOut.Erase();
                    scanf("%d", &Square);
                    Square--;
                    W = TryMove( Square );
                } while ( Square == -1 );
            }
        }
    }
    Print();
    puts( aStdOut.GetBuffer() );
    aStdOut.Erase();
}
#endif  // TICTACTOE_STDOUT


#ifdef TICTACTOE_MAIN
int main()
{
    char Answer[80];

    printf("Welcome to Tic-Tac-Toe!\n\n");
    printf("Here is the board numbering:\n");
    printf(" 1 | 2 | 3\n");
    printf("---+---+---\n");
    printf(" 4 | 5 | 6\n");
    printf("---+---+---\n");
    printf(" 7 | 8 | 9\n");
    printf("\n");
//    printf("Computer plays X, you play O.\n");

    ScTicTacToe aTTT;
    ByteString aStr;
    aTTT.GetOutput( aStr );
    puts( aStr.GetBuffer() );

    do
    {
        printf("\nDo you want to move first? ");
        scanf("%s", Answer);
        aTTT.Initialize( toupper(Answer[0]) == 'Y' );
        aTTT.Game();
        printf("\nDo you want to play again? ");
        scanf("%s", Answer);
    } while (toupper(Answer[0]) == 'Y');

    return 0;
}
#endif  // TICTACTOE_MAIN

