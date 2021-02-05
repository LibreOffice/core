#!/usr/bin/env perl -w
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

use strict;
use warnings;

sub usage() {
    print STDERR <<EOF;
Usage: preset-definitions-to-shape-types.pl [ --drawingml-adj-names-data | --vml-shape-types-data ] <shapes> <text>

Converts presetShapeDefinitions.xml and presetTextWarpDefinitions.xml to a
.cxx that contains VML with the definitions of the shapes.  The result is
written to stdout.

<shapes> presetShapeDefinitions.xml (including the path to it)
<text>   presetTextWarpDefinitions.xml (including the path to it)
EOF
    exit 1;
}

sub show_call_stack
{
    my ( $path, $line, $subr );
    my $max_depth = 30;
    my $i = 1;
    print STDERR "--- Begin stack trace ---\n";
    while ( (my @call_details = (caller($i++))) && ($i<$max_depth) ) {
        print STDERR "$call_details[1] line $call_details[2] in function $call_details[3]\n";
    }
    print STDERR "--- End stack trace ---\n";
}

my $drawingml_adj_names_data = 0;
my $vml_shape_types_data = 0;
my $src_shapes = shift;
if ($src_shapes eq "--drawingml-adj-names-data") {
    $drawingml_adj_names_data = 1;
    $src_shapes = shift;
} elsif ($src_shapes eq "--vml-shape-types-data") {
    $vml_shape_types_data = 1;
    $src_shapes = shift;
}
my $src_text = shift;

usage() if ( !defined( $src_shapes ) || !defined( $src_text ) ||
             $src_shapes eq "-h" || $src_shapes eq "--help" ||
             !-f $src_shapes || !-f $src_text );

# Global variables
my @levels = ();
my $shape_name = "";
my $state = "";
my $path = "";
my $adjust = "";
my %adj_names;
my $max_adj_no = 0;
my @formulas = ();
my %variables = ();
my $ignore_this_shape = 0;
my $handles = "";
my $textboxrect = "";
my $last_pos_x = "";
my $last_pos_y = "";
my $no_stroke = 0;
my $no_fill = 0;
my $path_w = 1;
my $path_h = 1;
my @quadratic_bezier = ();

my %result_shapes = ();

my %shapes_ids = (
    0 => 'notPrimitive',
    1 => 'rectangle',
    2 => 'roundRectangle',
    3 => 'ellipse',
    4 => 'diamond',
    5 => 'triangle',
    6 => 'rtTriangle',
    7 => 'parallelogram',
    8 => 'trapezoid',
    9 => 'hexagon',
    10 => 'octagon',
    11 => 'plus',
    12 => 'star5',
    13 => 'rightArrow',
    14 => 'thickArrow', # should not be used
    15 => 'homePlate',
    16 => 'cube',
    17 => 'wedgeRoundRectCallout', # balloon
    18 => 'star16', # seal
    19 => 'arc',
    20 => 'line',
    21 => 'plaque',
    22 => 'can',
    23 => 'donut',
    24 => 'textPlain', # textSimple - FIXME MS Office 2007 converts these to textboxes with unstyled text, so is it actually correct to map it to a real style?
    25 => 'textStop', # textOctagon FIXME see 24
    26 => 'textTriangle', # textHexagon FIXMME see 24
    27 => 'textCanDown', # textCurve FIXMME see 24
    28 => 'textWave1', # textWave FIXMME see 24
    29 => 'textArchUpPour', # textRing FIXMME see 24
    30 => 'textCanDown', # textOnCurve FIXMME see 24
    31 => 'textArchUp', # textOnRing FIXMME see 24
    32 => 'straightConnector1',
    33 => 'bentConnector2',
    34 => 'bentConnector3',
    35 => 'bentConnector4',
    36 => 'bentConnector5',
    37 => 'curvedConnector2',
    38 => 'curvedConnector3',
    39 => 'curvedConnector4',
    40 => 'curvedConnector5',
    41 => 'callout1',
    42 => 'callout2',
    43 => 'callout3',
    44 => 'accentCallout1',
    45 => 'accentCallout2',
    46 => 'accentCallout3',
    47 => 'borderCallout1',
    48 => 'borderCallout2',
    49 => 'borderCallout3',
    50 => 'accentBorderCallout1',
    51 => 'accentBorderCallout2',
    52 => 'accentBorderCallout3',
    53 => 'ribbon',
    54 => 'ribbon2',
    55 => 'chevron',
    56 => 'pentagon',
    57 => 'noSmoking',
    58 => 'star8', # seal8
    59 => 'star16', # seal16
    60 => 'star32', # seal32
    61 => 'wedgeRectCallout',
    62 => 'wedgeRoundRectCallout', # wedgeRRectCallout
    63 => 'wedgeEllipseCallout',
    64 => 'wave',
    65 => 'foldedCorner',
    66 => 'leftArrow',
    67 => 'downArrow',
    68 => 'upArrow',
    69 => 'leftRightArrow',
    70 => 'upDownArrow',
    71 => 'irregularSeal1',
    72 => 'irregularSeal2',
    73 => 'lightningBolt',
    74 => 'heart',
    75 => 'pictureFrame',
    76 => 'quadArrow',
    77 => 'leftArrowCallout',
    78 => 'rightArrowCallout',
    79 => 'upArrowCallout',
    80 => 'downArrowCallout',
    81 => 'leftRightArrowCallout',
    82 => 'upDownArrowCallout',
    83 => 'quadArrowCallout',
    84 => 'bevel',
    85 => 'leftBracket',
    86 => 'rightBracket',
    87 => 'leftBrace',
    88 => 'rightBrace',
    89 => 'leftUpArrow',
    90 => 'bentUpArrow',
    91 => 'bentArrow',
    92 => 'star24', # seal24
    93 => 'stripedRightArrow',
    94 => 'notchedRightArrow',
    95 => 'blockArc',
    96 => 'smileyFace',
    97 => 'verticalScroll',
    98 => 'horizontalScroll',
    99 => 'circularArrow',
    100 => 'notchedCircularArrow', # should not be used
    101 => 'uturnArrow',
    102 => 'curvedRightArrow',
    103 => 'curvedLeftArrow',
    104 => 'curvedUpArrow',
    105 => 'curvedDownArrow',
    106 => 'cloudCallout',
    107 => 'ellipseRibbon',
    108 => 'ellipseRibbon2',
    109 => 'flowChartProcess',
    110 => 'flowChartDecision',
    111 => 'flowChartInputOutput',
    112 => 'flowChartPredefinedProcess',
    113 => 'flowChartInternalStorage',
    114 => 'flowChartDocument',
    115 => 'flowChartMultidocument',
    116 => 'flowChartTerminator',
    117 => 'flowChartPreparation',
    118 => 'flowChartManualInput',
    119 => 'flowChartManualOperation',
    120 => 'flowChartConnector',
    121 => 'flowChartPunchedCard',
    122 => 'flowChartPunchedTape',
    123 => 'flowChartSummingJunction',
    124 => 'flowChartOr',
    125 => 'flowChartCollate',
    126 => 'flowChartSort',
    127 => 'flowChartExtract',
    128 => 'flowChartMerge',
    129 => 'flowChartOfflineStorage',
    130 => 'flowChartOnlineStorage',
    131 => 'flowChartMagneticTape',
    132 => 'flowChartMagneticDisk',
    133 => 'flowChartMagneticDrum',
    134 => 'flowChartDisplay',
    135 => 'flowChartDelay',
    136 => 'textPlain', # textPlainText
    137 => 'textStop',
    138 => 'textTriangle',
    139 => 'textTriangleInverted',
    140 => 'textChevron',
    141 => 'textChevronInverted',
    142 => 'textRingInside',
    143 => 'textRingOutside',
    144 => 'textArchUp', # textArchUpCurve
    145 => 'textArchDown', # textArchDownCurve
    146 => 'textCircle', # textCircleCurve
    147 => 'textButton', # textButtonCurve
    148 => 'textArchUpPour',
    149 => 'textArchDownPour',
    150 => 'textCirclePour',
    151 => 'textButtonPour',
    152 => 'textCurveUp',
    153 => 'textCurveDown',
    154 => 'textCascadeUp',
    155 => 'textCascadeDown',
    156 => 'textWave1',
    157 => 'textWave2',
    158 => 'textWave3',
    159 => 'textWave4',
    160 => 'textInflate',
    161 => 'textDeflate',
    162 => 'textInflateBottom',
    163 => 'textDeflateBottom',
    164 => 'textInflateTop',
    165 => 'textDeflateTop',
    166 => 'textDeflateInflate',
    167 => 'textDeflateInflateDeflate',
    168 => 'textFadeRight',
    169 => 'textFadeLeft',
    170 => 'textFadeUp',
    171 => 'textFadeDown',
    172 => 'textSlantUp',
    173 => 'textSlantDown',
    174 => 'textCanUp',
    175 => 'textCanDown',
    176 => 'flowChartAlternateProcess',
    177 => 'flowChartOffpageConnector',
    178 => 'callout1', # callout90
    179 => 'accentCallout1', # accentCallout90
    180 => 'borderCallout1', # borderCallout90
    181 => 'accentBorderCallout1', # accentBorderCallout90
    182 => 'leftRightUpArrow',
    183 => 'sun',
    184 => 'moon',
    185 => 'bracketPair',
    186 => 'bracePair',
    187 => 'star4', # seal4
    188 => 'doubleWave',
    189 => 'actionButtonBlank',
    190 => 'actionButtonHome',
    191 => 'actionButtonHelp',
    192 => 'actionButtonInformation',
    193 => 'actionButtonForwardNext',
    194 => 'actionButtonBackPrevious',
    195 => 'actionButtonEnd',
    196 => 'actionButtonBeginning',
    197 => 'actionButtonReturn',
    198 => 'actionButtonDocument',
    199 => 'actionButtonSound',
    200 => 'actionButtonMovie',
    201 => 'hostControl',
    202 => 'textBox'
);
# An error occurred, we have to ignore this shape
sub error( $ )
{
    my ( $msg ) = @_;

    $ignore_this_shape = 1;
    print STDERR "ERROR (in $shape_name ): $msg\n";
}

# Setup the %variables map with predefined values
sub setup_variables()
{
    %variables = (
        'l'        => 0,
        't'        => 0,
        'r'        => 21600,
        'b'        => 21600,

        'w'        => 21600,
        'h'        => 21600,
        'ss'       => 21600,
        'ls'       => 21600,

        'ssd2'     => 10800, # 1/2
        'ssd4'     => 5400,  # 1/4
        'ssd6'     => 3600,  # 1/6
        'ssd8'     => 2700,  # 1/8
        'ssd16'    => 1350,  # 1/16
        'ssd32'    => 675,   # 1/32

        'hc'       => 10800, # horizontal center
        'vc'       => 10800, # vertical center

        'wd2'      => 10800, # 1/2
        'wd3'      => 7200,  # 1/3
        'wd4'      => 5400,  # 1/4
        'wd5'      => 4320,  # 1/5
        'wd6'      => 3600,  # 1/6
        'wd8'      => 2700,  # 1/8
        'wd10'     => 2160,  # 1/10
        'wd12'     => 1800,  # 1/12
        'wd32'     => 675,   # 1/32

        'hd2'      => 10800, # 1/2
        'hd3'      => 7200,  # 1/3
        'hd4'      => 5400,  # 1/4
        'hd5'      => 4320,  # 1/5
        'hd6'      => 3600,  # 1/6
        'hd8'      => 2700,  # 1/8
        'hd10'     => 2160,  # 1/10
        'hd12'     => 1800,  # 1/12
        'hd32'     => 675,   # 1/32

        '25000'    => 5400,
        '12500'    => 2700,

        'cd4'      => 90,    # 1/4 of a circle
        'cd2'      => 180,   # 1/2 of a circle
        '3cd4'     => 270,   # 3/4 of a circle

        'cd8'      => 45,    # 1/8 of a circle
        '3cd8'     => 135,   # 3/8 of a circle
        '5cd8'     => 225,   # 5/8 of a circle
        '7cd8'     => 315,   # 7/8 of a circle

        '-5400000' => -90,
        '-10800000'=> -180,
        '-16200000'=> -270,
        '-21600000'=> -360,
        '-21599999'=> -360,

        '5400000'  => 90,
        '10800000' => 180,
        '16200000' => 270,
        '21600000' => 360,
        '21599999' => 360
#
#        '21600000' => 360,   # angle conversions
#        '27000000' => 450,
#        '32400000' => 540,
#        '37800000' => 630
    );
}

# Convert the (predefined) value to a number
sub value( $ )
{
    my ( $val ) = @_;

    my $result = $variables{$val};
    return $result if ( defined( $result ) );

    return $val if ( $val =~ /^[0-9-]+$/ );

    error( "Unknown variable '$val'." );

    show_call_stack();
    return $val;
}

# Convert the DrawingML formula to a VML one
my %command_variables = (
    'w' => 'width',
    'h' => 'height',
    'r' => 'width',
    'b' => 'height'
);

# The same as value(), but some of the hardcoded values can have a name
sub command_value( $ )
{
    my ( $value ) = @_;

    return "" if ( $value eq "" );

    return $value if ( $value =~ /^@/ );

    my $command_val = $command_variables{$value};
    if ( defined( $command_val ) ) {
        return $command_val;
    }

    return value( $value );
}

# Insert the new formula to the list of formulas
# Creates the name if it's empty...
sub insert_formula( $$ )
{
    my ( $name, $fmla ) = @_;

    my $i = 0;
    foreach my $f ( @formulas ) {
        if ( $f eq $fmla ) {
            if ( $name ne "" ) {
                $variables{$name} = "@" . $i;
            }
            return "@" . $i;
        }
        ++$i;
    }

    if ( $name eq "" ) {
        $name = "@" . ( $#formulas + 1 );
    }

    $variables{$name} = "@" . ( $#formulas + 1 );
    push @formulas, $fmla;

    if ( $#formulas > 127 ) {
        error( "Reached the maximum amount of formulas, have to ignore the shape '$shape_name'" );
    }

    return $variables{$name};
}

# The same as insert_formula(), but converts the params
sub insert_formula_params( $$$$$ )
{
    my ( $name, $command, $p1, $p2, $p3 ) = @_;

    my $result = $command;
    if ( $p1 ne "" ) {
        $result .= " " . command_value( $p1 );
        if ( $p2 ne "" ) {
            $result .= " " . command_value( $p2 );
            if ( $p3 ne "" ) {
                $result .= " " . command_value( $p3 );
            }
        }
    }

    return insert_formula( $name, $result );
}

# Convert the formula from DrawingML to VML
sub convert_formula( $$ )
{
    my ( $name, $fmla ) = @_;

    if ( $fmla =~ /^([^ ]+)/ ) {
        my $command = $1;

        # parse the parameters
        ( my $values = $fmla ) =~ s/^([^ ]+) *//;
        my $p1 = "";
	my $p2 = "";
	my $p3 = "";
        if ( $values =~ /^([^ ]+)/ ) {
            $p1 = $1;
            $values =~ s/^([^ ]+) *//;
            if ( $values =~ /^([^ ]+)/ ) {
                $p2 = $1;
                $values =~ s/^([^ ]+) *//;
                if ( $values =~ /^([^ ]+)/ ) {
                    $p3 = $1;
                }
            }
        }

        # now convert the formula
        if ( $command eq "+-" ) {
            if ( $p1 eq "100000" ) {
                $p1 = value( 'w' );
            }
            insert_formula_params( $name, "sum", $p1, $p2, $p3 );
            return;
        }
        elsif ( $command eq "*/" ) {
            if ( ( $p2 =~ /^(w|h|ss|hd2|wd2|vc)$/ ) && defined( $variables{$p1} ) ) {
                # switch it ;-) - presetTextWarpDefinitions.xml has it in other order
                my $tmp = $p1;
                $p1 = $p2;
                $p2 = $tmp;
            }

            if ( ( $p1 =~ /^(w|h|ss|hd2|wd2|vc)$/ ) && defined( $variables{$p2} ) ) {
                my $val3 = $p3;
                if ( $val3 =~ /^[0-9-]+$/ ) {
                    $val3 *= ( value( 'w' ) / value( $p1 ) );

                    # Oh yes, I'm too lazy to implement the full GCD here ;-)
                    if ( ( $val3 % 100000 ) == 0 ) {
                        $p1 = 1;
                        $p3 = sprintf( "%.0f", ( $val3 / 100000 ) );
                    }
                    elsif ( $val3 < 100000 ) {
                        $p3 = 1;
                        while ( ( ( $p3 * 100000 ) % $val3 ) != 0 ) {
                            ++$p3
                        }
                        $p1 = ( $p3 * 100000 ) / $val3;
                    }
                    else {
                        error( "Need to count the greatest common divisor." );
                    }
                }
            }
            elsif ( $p3 eq "100000" && $p2 =~ /^[0-9-]+$/ ) {
                # prevent overflows in some shapes
                $p2 = sprintf( "%.0f", ( $p2 / 10 ) );
                $p3 /= 10;
            }
            elsif ( $p3 eq "32768" && $p2 =~ /^[0-9-]+$/ ) {
                # prevent overflows in some shapes
                $p2 = sprintf( "%.0f", ( $p2 / 8 ) );
                $p3 /= 8;
            }
            elsif ( $p3 eq "50000" ) {
                $p3 = 10800;
            }
            elsif ( $name =~ /^maxAdj/ ) {
                my $val = value( $p1 );
                if ( $val =~ /^[0-9-]+$/ ) {
                    $p1 = sprintf( "%.0f", ( value( 'w' ) * $val / 100000 ) );
                }
            }

            if ( ( value( $p1 ) eq value( $p3 ) ) || ( value( $p2 ) eq value( $p3 ) ) ) {
                my $val = value( ( value( $p1 ) eq value( $p3 ) )? $p2: $p1 );
                if ( $val =~ /^@([0-9]+)$/ ) {
                    insert_formula( $name, $formulas[$1] );
                }
                else {
                    insert_formula( $name, "val $val" );
                }
            }
            else {
                insert_formula_params( $name, "prod", $p1, $p2, $p3 );
            }
            return;
        }
        elsif ( $command eq "+/" ) {
            # we have to split this into 2 formulas - 'sum' and 'prod'
            my $constructed = insert_formula_params( "", "sum", $p1, $p2, "0" );
            insert_formula_params( $name, "prod", 1, $constructed, $p3); # references the 'sum' formula
            return;
        }
        elsif ( $command eq "?:" ) {
            insert_formula_params( $name, "if", $p1, $p2, $p3 );
            return;
        }
        elsif ( $command eq "sin" || $command eq "cos" ) {
            if ( $p2 =~ /^[0-9-]+$/ && ( ( $p2 % 60000 ) == 0 ) ) {
                $p2 /= 60000;
            }
            else {
                $p2 = insert_formula_params( "", "prod", "1", $p2, "60000" );
            }
            # we have to use 'sumangle' even for the case when $p2 is const
            # and theoretically could be written as such; but Word does not
            # accept it :-(
            my $conv = insert_formula_params( "", "sumangle", "0", $p2, "0" );

            $p2 = $conv;

            insert_formula_params( $name, $command, $p1, $p2, "" );
            return;
        }
        elsif ( $command eq "abs" ) {
            insert_formula_params( $name, $command, $p1, "", "" );
            return;
        }
        elsif ( $command eq "max" || $command eq "min" ) {
            insert_formula_params( $name, $command, $p1, $p2, "" );
            return;
        }
        elsif ( $command eq "at2" ) {
            insert_formula_params( $name, "atan2", $p1, $p2, "" );
            return;
        }
        elsif ( $command eq "cat2" ) {
            insert_formula_params( $name, "cosatan2", $p1, $p2, $p3 );
            return;
        }
        elsif ( $command eq "sat2" ) {
            insert_formula_params( $name, "sinatan2", $p1, $p2, $p3 );
            return;
        }
        elsif ( $command eq "sqrt" ) {
            insert_formula_params( $name, "sqrt", $p1, "", "" );
            return;
        }
        elsif ( $command eq "mod" ) {
            insert_formula_params( $name, "mod", $p1, $p2, $p3 );
            return;
        }
        elsif ( $command eq "val" ) {
            insert_formula_params( $name, "val", value( $p1 ), "", "" );
            return;
        }
        else {
            error( "Unknown formula '$name', '$fmla'." );
        }
    }
    else {
        error( "Cannot convert formula's command '$name', '$fmla'." );
    }
}

# There's no exact equivalent of 'arcTo' in VML, we have to do some special casing...
my %convert_arcTo = (
    '0' => {
        '90' => {
            'path' => 'qy',
            'op' => [ 'sum 0 __last_x__ __wR__', 'sum __hR__ __last_y__ 0' ],
        },
        '-90' => {
            'path' => 'qy',
            'op' => [ 'sum 0 __last_x__ __wR__', 'sum 0 __last_y__ __hR__' ],
        },
    },
    '90' => {
        '90' => {
            'path' => 'qx',
            'op' => [ 'sum 0 __last_x__ __wR__', 'sum 0 __last_y__ __hR__' ],
        },
        '-90' => {
            'path' => 'qx',
            'op' => [ 'sum __wR__ __last_x__ 0', 'sum 0 __last_y__ __hR__' ],
        },
    },
    '180' => {
        '90' => {
            'path' => 'qy',
            'op' => [ 'sum __wR__ __last_x__ 0', 'sum 0 __last_y__ __hR__' ],
        },
        '-90' => {
            'path' => 'qy',
            'op' => [ 'sum __wR__ __last_x__ 0', 'sum __hR__ __last_y__ 0' ],
        },
    },
    '270' => {
        '90' => {
            'path' => 'qx',
            'op' => [ 'sum __wR__ __last_x__ 0', 'sum __hR__ __last_y__ 0' ],
        },
        '-90' => {
            'path' => 'qx',
            'op' => [ 'sum 0 __last_x__ __wR__', 'sum __hR__ __last_y__ 0' ],
        },
    },
);

# Elliptic quadrant
# FIXME optimize so that we compute the const values when possible
sub elliptic_quadrant( $$$$ )
{
    my ( $wR, $hR, $stAng, $swAng ) = @_;

    if ( defined( $convert_arcTo{$stAng} ) && defined( $convert_arcTo{$stAng}{$swAng} ) ) {
        my $conv_path = $convert_arcTo{$stAng}{$swAng}{'path'};
        my $conv_op_ref = $convert_arcTo{$stAng}{$swAng}{'op'};

        $path .= "$conv_path";

        my $pos_x = $last_pos_x;
        my $pos_y = $last_pos_y;
        for ( my $i = 0; $i <= $#{$conv_op_ref}; ++$i ) {
            my $op = $conv_op_ref->[$i];

            $op =~ s/__last_x__/$last_pos_x/g;
            $op =~ s/__last_y__/$last_pos_y/g;
            $op =~ s/__wR__/$wR/g;
            $op =~ s/__hR__/$hR/g;

            my $fmla = insert_formula( "", $op );

            $path .= $fmla;

            # so far it's sufficient just to rotate the positions
            # FIXME if not ;-)
            $pos_x = $pos_y;
            $pos_y = $fmla;
        }
        $last_pos_x = $pos_x;
        $last_pos_y = $pos_y;
    }
    else {
        error( "Unhandled elliptic_quadrant(), input is ($wR, $hR, $stAng, $swAng)." );
    }
}

# Convert the quadratic bezier to cubic (exact)
# No idea why, but the 'qb' did not work for me :-(
sub quadratic_to_cubic_bezier( $ )
{
    my ( $axis ) = @_;

    my $a0 = $quadratic_bezier[0]->{$axis};
    my $a1 = $quadratic_bezier[1]->{$axis};
    my $a2 = $quadratic_bezier[2]->{$axis};

    my $b0 = $a0;

    # $b1 = $a0 + 2/3 * ( $a1 - $a0 ), but in VML
    # FIXME optimize for constants - compute directly
    my $b1_1 = insert_formula_params( "", "sum", "0", $a1, $a0 );
    my $b1_2 = insert_formula_params( "", "prod", "2", $b1_1, "3" );
    my $b1   = insert_formula_params( "", "sum", $a0, $b1_2, "0" );

    # $b2 = $b1 + 1/3 * ( $a2 - $a0 );
    # FIXME optimize for constants - compute directly
    my $b2_1 = insert_formula_params( "", "sum", "0", $a2, $a0 );
    my $b2_2 = insert_formula_params( "", "prod", "1", $b2_1, "3" );
    my $b2   = insert_formula_params( "", "sum", $b1, $b2_2, "0" );

    my $b3 = $a2;

    return ( $b0, $b1, $b2, $b3 );
}

# Extend $path by one more point
sub add_point_to_path( $$ )
{
    my ( $x, $y ) = @_;

    if ( $path =~ /[0-9]$/ && $x =~ /^[0-9-]/ ) {
        $path .= ",";
    }
    $path .= $x;

    if ( $path =~ /[0-9]$/ && $y =~ /^[0-9-]/ ) {
        $path .= ",";
    }
    $path .= $y;
}

# Start of an element
sub start_element( $% )
{
    my ( $element, %attr ) = @_;

    push @levels, $element;

    #print "element: $element\n";

    if ( @levels > 1 && ( $levels[-2] eq "presetShapeDefinitons" ||
			  $levels[-2] eq "presetTextWarpDefinitions" ) ) {
        $shape_name = $element;

        $state = "";
        $ignore_this_shape = 0;
        $path = "";
        $adjust = "";
        $max_adj_no = 0;
        @formulas = ();
        $handles = "";
        $textboxrect = "";
        $last_pos_x = "";
        $last_pos_y = "";
        $no_stroke = 0;
        $no_fill = 0;
        @quadratic_bezier = ();

        setup_variables();

        if ( $shape_name eq "sun" ) {
            # hack for this shape
            $variables{'100000'} = "21600";
            $variables{'50000'} = "10800";
            $variables{'25000'} = "5400";
            $variables{'12500'} = "2700";
            $variables{'3662'} = "791";
        }

        my $found = 0;
        foreach my $name ( values( %shapes_ids ) ) {
            if ( $name eq $shape_name ) {
                $found = 1;
                last;
            }
        }
        if ( !$found ) {
            error( "Unknown shape '$shape_name'." );
        }
    }
    elsif ( $element eq "pathLst" ) {
        $state = "path";
    }
    elsif ( $element eq "avLst" ) {
        $state = "adjust";
    }
    elsif ( $element eq "gdLst" ) {
        $state = "formulas";
    }
    elsif ( $element eq "ahLst" ) {
        $state = "handles";
    }
    elsif ( $element eq "rect" ) {
        $textboxrect = value( $attr{'l'} ) . "," . value( $attr{'t'} ) . "," .
                       value( $attr{'r'} ) . "," . value( $attr{'b'} );
    }
    elsif ( $state eq "path" ) {
        if ( $element eq "path" ) {
            $no_stroke = ( defined( $attr{'stroke'} ) && $attr{'stroke'} eq 'false' );
            $no_fill = ( defined( $attr{'fill'} ) && $attr{'fill'} eq 'none' );
            $path_w = $attr{'w'};
            $path_h = $attr{'h'};
        }
        elsif ( $element eq "moveTo" ) {
            $path .= "m";
        }
        elsif ( $element eq "lnTo" ) {
            $path .= "l";
        }
        elsif ( $element eq "cubicBezTo" ) {
            $path .= "c";
        }
        elsif ( $element eq "quadBezTo" ) {
            my %points = ( 'x' => $last_pos_x, 'y' => $last_pos_y );
            @quadratic_bezier = ( \%points );
        }
        elsif ( $element eq "close" ) {
            $path .= "x";
        }
        elsif ( $element eq "pt" ) {
            # remember the last position for the arcTo
            $last_pos_x = value( $attr{'x'} );
            $last_pos_y = value( $attr{'y'} );

            $last_pos_x *= ( value( 'w' ) / $path_w ) if ( defined( $path_w ) );
            $last_pos_y *= ( value( 'h' ) / $path_h ) if ( defined( $path_h ) );

            if ( $#quadratic_bezier >= 0 ) {
                my %points = ( 'x' => $last_pos_x, 'y' => $last_pos_y );
                push( @quadratic_bezier, \%points );
            }
            else {
                add_point_to_path( $last_pos_x, $last_pos_y );
            }
        }
        elsif ( ( $element eq "arcTo" ) && ( $last_pos_x ne "" ) && ( $last_pos_y ne "" ) ) {
            # there's no exact equivalent of arcTo in VML, so we have to
            # compute here a bit...
            my $stAng = value( $attr{'stAng'} );
            my $swAng = value( $attr{'swAng'} );
            my $wR = value( $attr{'wR'} );
            my $hR = value( $attr{'hR'} );

            $wR *= ( value( 'w' ) / $path_w ) if ( defined( $path_w ) );
            $hR *= ( value( 'h' ) / $path_h ) if ( defined( $path_h ) );

            if ( ( $stAng =~ /^[0-9-]+$/ ) && ( $swAng =~ /^[0-9-]+$/ ) ) {
                if ( ( ( $stAng % 90 ) == 0 ) && ( ( $swAng % 90 ) == 0 ) && ( $swAng != 0 ) ) {
                    my $end = $stAng + $swAng;
                    my $step = ( $swAng > 0 )? 90: -90;

                    for ( my $cur = $stAng; $cur != $end; $cur += $step ) {
                        elliptic_quadrant( $wR, $hR, ( $cur % 360 ), $step );
                    }
                }
                else {
                    error( "Unsupported numeric 'arcTo' ($attr{'wR'}, $attr{'hR'}, $stAng, $swAng)." );
                }
            }
            else {
                error( "Unsupported 'arcTo' conversion ($attr{'wR'}, $attr{'hR'}, $stAng, $swAng)." );
            }
        }
        else {
            error( "Unhandled path element '$element'." );
        }
    }
    elsif ( $state eq "adjust" ) {
        if ( $element eq "gd" ) {
            my $adj_no = $attr{'name'};

            # Save this adj number for this type for later use.
            push(@{$adj_names{$shape_name}}, $adj_no);

            my $is_const = 0;

            $adj_no =~ s/^adj//;
            if ( $adj_no eq "" ) {
                $max_adj_no = 0;
            }
            elsif ( !( $adj_no =~ /^[0-9]*$/ ) ) {
                ++$max_adj_no;
                $is_const = 1;
            }
            elsif ( $adj_no != $max_adj_no + 1 ) {
                error( "Wrong order of adj values." );
                ++$max_adj_no;
            }
            else {
                $max_adj_no = $adj_no;
            }

            if ( $attr{'fmla'} =~ /^val ([0-9-]*)$/ ) {
                my $val = sprintf( "%.0f", ( 21600 * $1 ) / 100000 );
                if ( $is_const ) {
                    $variables{$adj_no} = $val;
                }
                elsif ( $adjust eq "" ) {
                    $adjust = $val;
                }
                else {
                    $adjust = "$val,$adjust";
                }
            }
            else {
                error( "Wrong fmla '$attr{'fmla'}'." );
            }
        }
        else {
            error( "Unhandled adjust element '$element'." );
        }
    }
    elsif ( $state eq "formulas" ) {
        if ( $element eq "gd" ) {
            if ( $attr{'fmla'} =~ /^\*\/ (h|w|ss) adj([0-9]+) 100000$/ ) {
                insert_formula( $attr{'name'}, "val #" . ( $max_adj_no - $2 ) );
            }
            elsif ( $attr{'fmla'} =~ /^pin [^ ]+ ([^ ]+) / ) {
                print STDERR "TODO Map 'pin' to VML as xrange for handles.\n";
                my $pin_val = $1;
                if ( $pin_val eq "adj" ) {
                    insert_formula( $attr{'name'}, "val #0" );
                }
                elsif ( $pin_val =~ /^adj([0-9]+)/ ) {
                    insert_formula( $attr{'name'}, "val #" . ( $max_adj_no - $1 ) );
                }
                else {
                    insert_formula( $attr{'name'}, "val " . value( $pin_val ) );
                }
            }
            elsif ( $attr{'fmla'} =~ /adj/ ) {
                error( "Non-standard usage of adj in '$attr{'fmla'}'." );
            }
            else {
                convert_formula( $attr{'name'}, $attr{'fmla'} );
            }
        }
    }
    elsif ( $state eq "handles" ) {
        if ( $element eq "pos" ) {
            $handles .= "<v:h position=\"" . value( $attr{'x'} ) . "," . value( $attr{'y'} ) . "\"/>\n";
        }
    }
}

# End of an element
sub end_element( $ )
{
    my ( $element ) = @_;

    pop @levels;

    if ( $element eq $shape_name ) {
        if ( !$ignore_this_shape ) {
            # we have all the info, generate the shape now
            $state = "";

            # shape path
            my $out = "<v:shapetype id=\"_x0000_t__ID__\" coordsize=\"21600,21600\" o:spt=\"__ID__\" ";
            if ( $adjust ne "" ) {
                $out .= "adj=\"$adjust\" ";
            }

            # optimize it [yes, we need this twice ;-)]
            $path =~ s/([^0-9-@])0([^0-9-@])/$1$2/g;
            $path =~ s/([^0-9-@])0([^0-9-@])/$1$2/g;

            $out .= "path=\"$path\">\n";

            # stroke
            $out .= "<v:stroke joinstyle=\"miter\"/>\n";

            # formulas
            if ( $#formulas >= 0 )
            {
                $out .= "<v:formulas>\n";
                foreach my $fmla ( @formulas ) {
                    $out .= "<v:f eqn=\"$fmla\"/>\n"
                }
                $out .= "</v:formulas>\n";
            }

            # path
            if ( $textboxrect ne "" ) { # TODO connectlocs, connectangles
                $out .= "<v:path gradientshapeok=\"t\" o:connecttype=\"rect\" textboxrect=\"$textboxrect\"/>\n";
            }

            # handles
            if ( $handles ne "" ) {
                $out .= "<v:handles>\n$handles</v:handles>\n";
            }

            $out .="</v:shapetype>";

            # hooray! :-)
            $result_shapes{$shape_name} = $out;
        }
        else {
            print STDERR "Shape '$shape_name' ignored; see the above error(s) for the reason.\n";
        }
        $shape_name = "";
    }
    elsif ( $state eq "path" ) {
        if ( $element eq "path" ) {
            $path .= "ns" if ( $no_stroke );
            $path .= "nf" if ( $no_fill );
            $path .= "e";
        }
        elsif ( $element eq "quadBezTo" ) {
            # we have to convert the quadratic bezier to cubic
            if ( $#quadratic_bezier == 2 ) {
                my @points_x = quadratic_to_cubic_bezier( 'x' );
                my @points_y = quadratic_to_cubic_bezier( 'y' );

                $path .= "c";
                # ignore the starting point
                for ( my $i = 1; $i < 4; ++$i ) {
                    add_point_to_path( $points_x[$i], $points_y[$i] );
                }
            }
            else {
                error( "Wrong number of points of the quadratic bezier." );
            }
            @quadratic_bezier = ();
        }
    }
    elsif ( $element eq "avLst" ) {
        $state = "";
    }
    elsif ( $element eq "gdLst" ) {
        $state = "";
    }
    elsif ( $element eq "ahLst" ) {
        $state = "";
    }
}

# Text inside an element
sub characters( $ )
{
    #my ( $text ) = @_;
}

#################### A trivial XML parser ####################

# Parse the attributes
sub parse_start_element( $ )
{
    # split the string containing both the elements and attributes
    my ( $element_tmp ) = @_;

    $element_tmp =~ s/\s*$//;
    $element_tmp =~ s/^\s*//;

    ( my $element = $element_tmp ) =~ s/\s.*$//;
    if ( $element_tmp =~ /\s/ ) {
        $element_tmp =~ s/^[^\s]*\s//;
    }
    else {
        $element_tmp = "";
    }

    # we have the element, now the attributes
    my %attr;
    my $is_key = 1;
    my $key = "";
    foreach my $tmp ( split( /"/, $element_tmp ) ) {
        if ( $is_key ) {
            $key = $tmp;
            $key =~ s/^\s*//;
            $key =~ s/\s*=\s*$//;
        }
        else {
            $attr{$key} = $tmp;
        }
        $is_key = !$is_key;
    }

    if ( $element ne "" ) {
        start_element( $element, %attr );
    }
}

# Parse the file
sub parse( $ )
{
    my ( $file ) = @_;

    my $in_comment = 0;
    my $line = "";
    while (<$file>) {
        # ignore comments
        s/<\?[^>]*\?>//g;
        s/<!--[^>]*-->//g;
        if ( /<!--/ ) {
            $in_comment = 1;
            s/<!--.*//;
        }
        elsif ( /-->/ && $in_comment ) {
            $in_comment = 0;
            s/.*-->//;
        }
        elsif ( $in_comment ) {
            next;
        }
        # ignore empty lines
        chomp;
        s/^\s*//;
        s/\s*$//;
        next if ( $_ eq "" );

        # take care of lines where element continues
        if ( $line ne "" ) {
            $line .= " " . $_;
        }
        else {
            $line = $_;
        }
        next if ( !/>$/ );

        # the actual parsing
        my @starts = split( /</, $line );
        $line = "";
        foreach my $start ( @starts ) {
            next if ( $start eq "" );

            my @ends = split( />/, $start );
            my $element = $ends[0];
            my $data = $ends[1];

            # start or end element
            if ( $element =~ /^\/(.*)/ ) {
                end_element( $1 );
            }
            elsif ( $element =~ /^(.*)\/$/ ) {
                parse_start_element( $1 );
                ( my $end = $1 ) =~ s/\s.*$//;
                end_element( $end );
            }
            else {
                parse_start_element( $element );
            }

            # the data
            characters( $data ) if ( defined( $data ) && $data ne "" );
        }
    }
}

# Do the real work
my $file;
open( $file, "<$src_shapes" ) || die "Cannot open $src_shapes: $!";
parse( $file );
close( $file );

open( $file, "<$src_text" ) || die "Cannot open $src_text: $!";
parse( $file );
close( $file );

if ( !defined( $result_shapes{'textBox'} ) ) {
    # tdf#114842 shapetype id of the textbox, must be the same as defined
    $result_shapes{'textBox'} =
        "<v:shapetype id=\"_x0000_t__ID__\" coordsize=\"21600,21600\" " .
        "o:spt=\"__ID__\" path=\"m,l,21600l21600,21600l21600,xe\">\n" .
        "<v:stroke joinstyle=\"miter\"/>\n" .
        "<v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/>\n" .
        "</v:shapetype>";
}

# Generate the data
if ($drawingml_adj_names_data eq 1) {
    foreach my $adj_name (sort(keys %adj_names))
    {
        foreach my $adj (@{$adj_names{$adj_name}})
        {
            print "$adj_name\t$adj\n";
        }
    }
    exit 0;
} elsif ($vml_shape_types_data eq 1) {
    for ( my $i = 0; $i < 203; ++$i ) {
        if ( $i < 4 ) {
            print "/* $i - $shapes_ids{$i} - handled separately */\nNULL\n";
        }
        else {
            print "/* $i - $shapes_ids{$i} */\n";
            my $out = $result_shapes{$shapes_ids{$i}};
            if ( defined( $out ) ) {
                # set the id
                $out =~ s/__ID__/$i/g;

                # output as string
                $out =~ s/\n//g;

                print "$out\n";
            }
            else {
                print "NULL\n";
            }
        }
    }
    exit 0;
}

# should not happen
exit 1;

# vim:set ft=perl shiftwidth=4 softtabstop=4 expandtab: #
