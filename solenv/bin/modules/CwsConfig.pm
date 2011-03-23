#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


#
# CwsConfig.pm - package for read CWS config data
#

package CwsConfig;
use strict;

use Carp;
use URI::Escape;

##### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{_CONFIG_FILE}        = undef;    # config file
    $self->{_GLOBAL}             = undef;    # is it a global config file?
    $self->{VCSID}               = undef;    # VCSID
    $self->{CWS_DB_URL_LIST_REF} = undef;    # list of CWS DB servers
    $self->{NET_PROXY}           = undef;    # network proxy
    $self->{CWS_SERVER_ROOT}     = undef;    # cvs server
    $self->{CWS_MIRROR_ROOT}     = undef;    # mirror of cvs server
    $self->{CWS_LOCAL_ROOT}      = undef;    # local cvs server
    $self->{PUBLIC_SVN_SERVER}   = undef;    # public svn server
    $self->{PRIVATE_SVN_SERVER}  = undef;    # private svn server
    bless ($self, $class);
    return $self;
}

sub vcsid
{
    my $self = shift;

    if ( !defined($self->{VCSID}) ) {
        # environment overrides config file
        my $vcsid = $ENV{VCSID};
        if ( !defined($vcsid) ) {
            # check config file
            my $config_file = $self->get_config_file();
            $vcsid = $config_file->{CWS_CONFIG}->{'CVS_ID'};
            if ( !defined($vcsid) ) {
                # give up
                croak("ERROR: no CVS_ID entry found in '\$HOME/.cwsrc'.\n" );
            }
        }
        $self->{VCSID} = $vcsid;
    }
    return $self->{VCSID};
}

sub cws_db_url_list_ref
{
    my $self = shift;

    if ( !defined($self->{CWS_DB_URL_LIST_REF}) ) {
        my $config_file = $self->get_config_file();

        my $i = 1;
        my @cws_db_servers;

        while ( 1 ) {
            my $val = $config_file->{CWS_CONFIG}->{"CWS_DB_SERVER_$i"};
            last if !defined($val);
            push(@cws_db_servers, $val);
            $i++;
        }

        if ( !@cws_db_servers) {
            croak("ERROR: no CWS_DB_SERVER_* entry found in '\$HOME/.cwsrc'.\n" );
        }

        if ( $cws_db_servers[0] =~ /^https:\/\// ) {
            my $id = $self->vcsid();
            my $password = $config_file->{CWS_CONFIG}->{'CVS_PASSWORD'};

            if ( !defined($password) ) {
                croak("ERROR: no CVS_PASSWORD entry found in '\$HOME/.cwsrc'.\n" );
            }

            # *i49473* - do not accept scrambled passwords ending with a space
            if ( $password =~ / $/) {
                croak("ERROR: The (scrambled) CVS_PASSWORD ends with a space. This is known to cause problems when connecting to the OpenOffice.org EIS database. Please change your OOo account's password" );
            }

            # We are going to stuff $id and $password in an URL, do proper escaping.
            $id = uri_escape($id);
            $password = uri_escape($password);

            foreach ( @cws_db_servers ) {
                s/^https:\/\//https:\/\/$id:$password@/;
            }
        }

        $self->{CWS_DB_URL_LIST_REF} = \@cws_db_servers;
    }
    return $self->{CWS_DB_URL_LIST_REF};
}

sub net_proxy
{
    my $self = shift;

    if ( !defined($self->{NET_PROXY}) ) {
        my $config_file = $self->get_config_file();
        my $net_proxy = $config_file->{CWS_CONFIG}->{'PROXY'};
        if ( !defined($net_proxy) ) {
            $net_proxy = "";
        }
        $self->{NET_PROXY} = $net_proxy;
    }
    return $self->{NET_PROXY} ? $self->{NET_PROXY} : undef;
}

sub cvs_binary
{
    my $self = shift;

    if ( !defined($self->{CVS_BINARY}) ) {
        my $config_file = $self->get_config_file();
        my $cvs_binary = $config_file->{CWS_CONFIG}->{'CVS_BINARY'};
        if ( !defined($cvs_binary) ) {
            # defaults
            $cvs_binary = ($^O eq 'MSWin32') ? 'cvs.exe' : 'cvs';
        }
        # special case, don't ask
        if ( $self->{_GLOBAL} && $cvs_binary =~ /cvs.clt2/ && $^O eq 'MSWin32' ) {
            $cvs_binary = 'cvsclt2.exe';
        }
        $self->{CVS_BINARY} = $cvs_binary;
    }
    return $self->{CVS_BINARY};
}

sub cvs_server_root
{
    my $self = shift;

    if ( !defined($self->{CVS_SERVER_ROOT}) ) {
        my $config_file = $self->get_config_file();
        my $cvs_server_root = $config_file->{CWS_CONFIG}->{'CVS_SERVER_ROOT'};
        if ( !defined($cvs_server_root) ) {
            # give up, this is a mandatory entry
            croak("ERROR: can't parse CVS_SERVER_ROOT entry in '\$HOME/.cwsrc'.\n");
        }
        if ( $self->{_GLOBAL} ) {
            # a global config file will almost always have the wrong vcsid in
            # the cvsroot -> substitute vcsid
            my $id = $self->vcsid();
            $cvs_server_root =~ s/:pserver:\w+@/:pserver:$id@/;
        }
        $self->{CVS_SERVER_ROOT} = $cvs_server_root;
    }
    return $self->{CVS_SERVER_ROOT};
}

sub cvs_mirror_root
{
    my $self = shift;

    if ( !defined($self->{CVS_MIRROR_ROOT}) ) {
        my $config_file = $self->get_config_file();
        my $cvs_mirror_root = $config_file->{CWS_CONFIG}->{'CVS_MIRROR_ROOT'};
        if ( !defined($cvs_mirror_root) ) {
            $cvs_mirror_root = "";
        }
        $self->{CVS_MIRROR_ROOT} = $cvs_mirror_root;
    }
    return $self->{CVS_MIRROR_ROOT} ? $self->{CVS_MIRROR_ROOT} : undef;
}

sub cvs_local_root
{
    my $self = shift;

    if ( !defined($self->{CVS_LOCAL_ROOT}) ) {
        my $config_file = $self->get_config_file();
        my $cvs_local_root = $config_file->{CWS_CONFIG}->{'CVS_LOCAL_ROOT'};
        if ( !defined($cvs_local_root) ) {
            $cvs_local_root = "";
        }
        $self->{CVS_LOCAL_ROOT} = $cvs_local_root;
    }
    return $self->{CVS_LOCAL_ROOT} ? $self->{CVS_LOCAL_ROOT} : undef;
}

sub get_cvs_server
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_server_root(), 'SERVER');
    return $server;
}

sub get_cvs_mirror
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_mirror_root(), 'MIRROR');
    return $server;
}

sub get_cvs_local
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_local_root(), 'LOCAL');
    return $server;
}

sub get_cvs_server_method
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_server_root(), 'SERVER');
    return $method;
}

sub get_cvs_mirror_method
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_mirror_root(), 'MIRROR');
    return $method;
}

sub get_cvs_local_method
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_local_root(), 'LOCAL');
    return $method;
}

sub get_cvs_server_repository
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_server_root(), 'SERVER');
    return $repository;
}

sub get_cvs_mirror_repository
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_mirror_root(), 'MIRROR');
    return $repository;
}

sub get_cvs_local_repository
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_local_root(), 'LOCAL');
    return $repository;
}

sub get_cvs_server_id
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_server_root(), 'SERVER');
    return $id;
}

sub get_cvs_mirror_id
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_mirror_root(), 'MIRROR');
    return $id;
}

sub get_cvs_local_id
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_local_root(), 'LOCAL');
    return $id;
}

#### SVN methods ####

sub get_ooo_svn_server
{
    my $self = shift;

    if ( !defined($self->{SVN_SERVER}) ) {
        my $config_file = $self->get_config_file();
        my $ooo_svn_server = $config_file->{CWS_CONFIG}->{'SVN_SERVER'};
        if ( !defined($ooo_svn_server) ) {
            $ooo_svn_server = "";
        }
        $self->{SVN_SERVER} = $ooo_svn_server;
    }
    return $self->{SVN_SERVER} ? $self->{SVN_SERVER} : undef;
}

sub get_so_svn_server
{
    my $self = shift;

    if ( !defined($self->{SO_SVN_SERVER}) ) {
        my $config_file = $self->get_config_file();
        my $so_svn_server = $config_file->{CWS_CONFIG}->{'SO_SVN_SERVER'};
        if ( !defined($so_svn_server) ) {
            $so_svn_server = "";
        }
        $self->{SO_SVN_SERVER} = $so_svn_server;
    }
    return $self->{SO_SVN_SERVER} ? $self->{SO_SVN_SERVER} : undef;
}

#### HG methods ####

sub _get_hg_source
{
    my $self               = shift;
    my $repository_source  = shift;
    if ( !defined($self->{$repository_source}) ) {
        my $config_file = $self->get_config_file();
        my $source = $config_file->{CWS_CONFIG}->{$repository_source};
        if ( !defined($source) ) {
            $source = "";
        }
        $self->{$repository_source} = $source;
    }
    return $self->{$repository_source} ? $self->{$repository_source} : undef;

}

sub get_hg_source
{
    my $self        = shift;
    my $repository  = shift;
    my $location    = shift;

    #Special prefix handling, see cwsrc
    if ($repository eq "OOO")
    {
        if ($location eq "LOCAL")
        {
            return $self->_get_hg_source('HG_LOCAL_SOURCE');
        }
        elsif ($location eq "LAN")
        {
            return $self->_get_hg_source('HG_LAN_SOURCE');
        }
        elsif ($location eq "REMOTE")
        {
            return $self->_get_hg_source('HG_REMOTE_SOURCE');
        }
    }
    else
    {
        if ($location eq "LOCAL")
        {
            return $self->_get_hg_source($repository.'_HG_LOCAL_SOURCE');
        }
        elsif ($location eq "LAN")
        {
            return $self->_get_hg_source($repository.'_HG_LAN_SOURCE');
        }
        elsif ($location eq "REMOTE")
        {
            return $self->_get_hg_source($repository.'_HG_REMOTE_SOURCE');
        }
    }
}

#### Prebuild binaries configuration ####

sub get_prebuild_binaries_location
{
    my $self = shift;

    if ( !defined($self->{PREBUILD_BINARIES}) ) {
        my $config_file = $self->get_config_file();
        my $pre_build_binaries = $config_file->{CWS_CONFIG}->{'PREBUILD_BINARIES'};
        if ( !defined($pre_build_binaries) ) {
            $pre_build_binaries = "";
        }
        $self->{PREBUILD_BINARIES} = $pre_build_binaries;
    }
    return $self->{PREBUILD_BINARIES} ? $self->{PREBUILD_BINARIES} : undef;
}



#### class methods #####
sub get_config
{
    my $config = CwsConfig->new();
    return $config;
}

sub split_root
{
    my $root = shift;
    my $type = shift;

    if ( !defined($root) ) {
        return (undef, undef, undef, undef);
    }

    my ($dummy, $method, $id_at_host, $repository) = split(/:/, $root);
    $repository =~ s/^\d*//;
    my ($id, $server);
    if ( $id_at_host ) {
        ($id, $server) = split(/@/, $id_at_host);
    }
    if ( !defined($method) || !defined($id) || !defined($server) || !defined($repository) ) {
        # give up
        print  "$method, $id, $server, $repository\n";
        croak("ERROR: can't parse CVS_".$type."_ROOT entry in '\$HOME/.cwsrc'.\n");
    }
    return ($method, $id, $server, $repository);
}

#### private helper methods ####

sub get_config_file
{
    my $self = shift;

    if ( !defined $self->{_CONFIG_FILE} ) {
        $self->parse_config_file();
    }
    return $self->{_CONFIG_FILE};
}

sub read_config
{
    my $self = shift;
    my $fname = shift;
    my $fhandle;
    my $section = '';
    my %config;

    open ($fhandle, $fname) || croak("ERROR: Can't open '$fname': $!");
    while ( <$fhandle> ) {
        tr/\r\n//d;   # win32 pain
        # Issue #i62815#: Scrambled CVS passwords may contain one or more '#'.
        # Ugly special case needed: still allow in-line (perl style) comments
        # elsewhere because existing configuration files may depend on them.
        if ( !/^\s*CVS_PASSWORD/ ) {
            s/\#.*//; # kill comments
        }
        /^\s*$/ && next;

        if (/\[\s*(\S+)\s*\]/) {
            $section = $1;
            if (!defined $config{$section}) {
                $config{$section} = {};
            }
        }
        defined $config{$section} || croak("ERROR: unknown / no section '$section'\n");
        if ( m/(\w[\w\d]*)=(.*)/ ) {
            my $var = $1;
            my $val = $2;
            # New style value strings may be surrounded by quotes
            if ( $val =~ s/\s*(['"])(.*)\1\s*$/$2/ ) {
                my $quote = $1;
                # If and only if the value string is surrounded by quotes we
                # can expect that \" or \' are escaped characters. In an unquoted
                # old style value string they could mean exactly what is standing there
                #
                # Actually the RE above works without quoting the quote character
                # (either " or ') inside the value string but users will probably
                # expect that they need to be escaped if quotes are used.
                #
                # This is still not completly correct for all thinkable situations but
                # should be good enough for all practical use cases.
                $val =~ s/\\($quote)/$1/g;
            }
            $config{$section}->{$var} = $val;
            # print "Set '$var' to '$val'\n";
        }
    }
    close ($fhandle) || croak("ERROR: Failed to close: $!");

    $self->{_CONFIG_FILE} = \%config;
}

sub parse_config_file
{
    my $self = shift;

    my $config_file;
    # check for config files
    if ( -e "$ENV{HOME}/.cwsrc" ) {
    $self->read_config("$ENV{HOME}/.cwsrc");
        $self->{_GLOBAL} = 0;
    }
    elsif ( -e "$ENV{COMMON_ENV_TOOLS}/cwsrc" ) {
        $self->read_config("$ENV{COMMON_ENV_TOOLS}/cwsrc");
        $self->{_GLOBAL} = 1;
    }
    else {
        croak("ERROR: can't find CWS config file '\$HOME/.cwsrc'.\n");
    }
}

sub sointernal
{
    my $self = shift;
    my $config_file = $self->get_config_file();
    my $val = ($config_file->{CWS_CONFIG}->{"SO_INTERNAL"}) ? 1 : 0;
    return $val;
}
1; # needed by "use" or "require"
