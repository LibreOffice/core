#
# Cvs.pm - package for manipulating CVS archives
#
# $Id: Cvs.pm,v 1.1.1.1 2000-09-20 14:43:11 hr Exp $

package Cvs;
use strict;

##### ctor ####

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $self = {};
    $self->{NAME} = undef;
    $self->{HEAD} = undef;
    $self->{FLAGS} = undef;
    if ($^O eq "MSWin32" || $^O eq "os2" ) {
    $self->{CVS_BINARY} = "_cvs.exe";
    } else {
    $self->{CVS_BINARY} = "cvs.clt";
    }
    $self->{REV_DATA} = {};
    $self->{REV_SORTED} = [];
    $self->{REV_TAGS} = {};
    $self->{TAGS} = {};
    $self->{"_PARSED"} = undef;
    $self->{"_SORTED"} = undef;
    $self->{"_TAGGED"} = undef;
    bless ($self, $class);
    return $self;
}

#### methods to access per object data ####

sub name {
    my $self = shift;
    if ( @_ ) {
    $self->{name} = shift;
    }
    return $self->{name};
}

sub cvs_binary {
    my $self = shift;
    if ( @_ ) {
    $self->{CVS_BINARY} = shift;
    }
    return $self->{CVS_BINARY};
}

sub get_data_by_rev {
    my $self = shift;
    $self->parse_log();

    return $self->{REV_DATA};
}

sub get_sorted_revs {
    my $self = shift;

    if ( $self->{"_SORTED"} ) {
    return $self->{REV_TAGS};
    }

    $self->parse_log();

    sub by_rev {
    # comparison function for sorting
    my (@field_a, @field_b, $min_field, $i);

    @field_a = split /\./, $a;
    @field_b = split /\./, $b;
    $min_field = ($#field_a > $#field_b) ? $#field_b : $#field_a;

    for ($i = 0; $i <= $min_field; $i++)
      {
          if ( ($field_a[$i] < $field_b[$i]) ) {
          return -1;
          }
          if ( ($field_a[$i] > $field_b[$i]) ) {
          return 1;
          }
      }

    if ( $#field_a == $#field_b ) {
        return 0;
    }
    # eg. 1.70 sorts before 1.70.1.0
    ($#field_a < $#field_b) ? return -1 : return 1;
    }


    @{$self->{REV_SORTED}} = sort by_rev (keys %{$self->{REV_DATA}});
    $self->{"_SORTED"} = 1;
    return $self->{REV_SORTED};
}

sub get_tags_by_rev {
    my $self = shift;
    my ($tag, $rev);

    if ( $self->{"_TAGGED"} ) {
    return $self->{REV_TAGS};
    }

    $self->parse_log();
    foreach $tag (keys %{$self->{TAGS}}) {
    $rev = $self->{TAGS}->{$tag};
    push (@{$self->{REV_TAGS}->{$rev}}, $tag);
    }

    $self->{"_TAGGED"} = 1;
    return $self->{REV_TAGS};
}

sub get_flags {
    my $self = shift;
    $self->parse_log();

    return $self->{FLAGS};
}

sub get_tags {
    my $self = shift;

    $self->parse_log();
    return $self->{TAGS};
}

sub get_head {
    my $self = shift;

    $self->parse_log();
    return $self->{HEAD};
}

#### methods to manipulate archive ####
sub delete_rev {
    my $self = shift;
    my $rev = shift;
    my $file = $self->name;
    my $out;

    if ($^O eq "MSWin32") {
    open (CVSDELETE,
          "$self->{CVS_BINARY} admin -o$rev $file 2>nul |");
    } elsif ( $^O eq "os2" ) {
    open (CVSDELETE,
          "$self->{CVS_BINARY} admin -o$rev $file 2>nul |");
    } else {
    open (CVSDELETE,
          "$self->{CVS_BINARY} admin -o$rev $file 2>/dev/null |");
    }
    while(<CVSDELETE>) {
    /deleting revision $rev/ && return 1;
    }
    close(CVSDELETE);
    return 0;
}

#### private methods ####
sub parse_log {
    my $self = shift;
    if ( $self->{"_PARSED"} ) {
    return;
    }
    my $file = $self->name;
    my $in_revisions = 0;
    my $in_tags = 0;
    my $rev_data = {};
    my ($rev, $date, $author, $state, $comment, @branches);

    open(CVSLOG, "$self->{CVS_BINARY} log $file |");

    while( <CVSLOG> ) {
    chomp;

    if ( $in_revisions ) {

        /revision\s((\d|\.)+)$/o && do { $rev = $1; next; };

        /^date:\s(\S+\s\S+);\s+author:\s(\S+);\s+state:\s(\S+);/
          && do { $date = $1; $author = $2; $state = $3; next; };

        /^branches:((\s+(\d|\.)+;)+)$/o && do {
        my $line;
        $line = $1;
        $line =~ s/\s//go;
        @branches = split(/;/, $line);
        next;
        };

        (/^----------------------------$/o || /^=============================================================================$/o)
          && do {
          $rev_data = {
                   DATE => $date,
                   AUTHOR => $author,
                   STATE => $state,
                   COMMENT => $comment,
                   BRANCHES => [ @branches ]
                  };
          $self->{REV_DATA}->{$rev} = $rev_data;
          $comment = undef;
          @branches = ();
          next;
          };

        $comment .= $_ . "\n" ;
    } elsif ( $in_tags ) {
        /^keyword\ssubstitution:/o
          && do { $self->{FLAGS} = $'; $in_tags--; next; };

        /^\t(\w+):\s((\d|\.)+)$/o
          && do { $self->{TAGS}->{$1} = $2; next; };

    } else {
        /^----------------------------$/o
          && do { $in_revisions++; next; };

        /^symbolic\snames:$/o && do { $in_tags++; next; };

        /^head:\s((\d|\.)+)$/o && do { $self->{HEAD} = $1; next; };
    }
    }

    close(CVSLOG);

    $self->{"_PARSED"} = 1;
}

####

1; # needed by "use" or "require"

