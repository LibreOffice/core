#*************************************************************************
#
#   $RCSfile: Eis.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2004-06-26 00:20:19 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************


#
# Eis.pm - package for accessing/manipulating the EIS database via SOAP
#

package Eis;
use strict;

use SOAP::Lite;
use Class::Struct;
use Carp;

# Declaration of class Eis together with ctor and accessors.
# See 'perldoc Class::Struct' for details

struct Eis => [
    # public members
    uri           => '$',           # name of webservice
    proxy_list    => '@',           # list of proxy URLs
    current_proxy => '$',           # current proxy (index in proxy_list)
    net_proxy     => '$',           # network proxy to pass through firewall
    # private members
    eis_connector => '$'            # SOAP connector to EIS database
];

#### public methods ####

# Any not predeclared method call to this package is
# interpreted as a SOAP method call. We use the AUTOLOAD
# mechanism to intercept these calls and delgate them
# to the eis_connector.
# See the 'Camel Book', 3rd edition, page 337 for an
# explanation of the AUTOLOAD mechanism.
sub AUTOLOAD
{
    my $self = shift;
    my $callee = $Eis::AUTOLOAD; # $callee now holds the name of
                                 # called subroutine
                                 #
    return if $callee =~ /::DESTROY$/;
    $callee = substr($callee, 5);

    my $sl = $self->eis_connector();
    if ( !$sl ) {
        $sl = $self->init_eis_connector();
        $self->eis_connector($sl);
    }

    my $response;
    while ( 1 )  {
        # Call callee() on web service.
        eval { $response = $sl->$callee(@_) };
        if ( $@ ) {
            # Transport error (server not available, timeout, etc).
            # Use backup server.
            print STDERR ("Warning: web service unavailable. Trying backup server.\n");
            if ( !$self->set_next_proxy() ) {
                # All proxies tried, out of luck
                carp("ERROR: Connection to EIS database failed.\n");
                return undef;
            }
        }
        else {
            last;
        }
    }

    if ( $response->fault() ) {
        my $fault_msg = get_soap_fault_message($response);
        die $fault_msg; # throw $fault_msg as exception
    }
    else {
        return $response->result();
    }
}

#### public class methods ####

# Turn scalar into SOAP string.
sub to_string
{
    my $value = shift;

    return SOAP::Data->type(string => $value);
}

#### non public instance methods ####

# Initialize SOAP connection to EIS.
sub init_eis_connector
{
    my $self = shift;

    # Init current_proxy with first element of the proxy list.
    my $current = $self->current_proxy(0);

    if ( !$self->uri() ) {
        carp("ERROR: web service URI not set.");
        return undef;
    }

    if ( !$self->proxy_list->[$current] ) {
        carp("ERROR: proxy list not proper initialized.");
        return undef;
    }

    # might be needed to get through a firewall
    if ( defined($self->net_proxy()) ) {
        $ENV{HTTPS_PROXY}=$self->net_proxy();
    }

    my $proxy = $self->proxy_list()->[$current];
    return create_eis_connector($self->uri(), $proxy);
}

# Advance one entry in proxy list.
sub set_next_proxy
{
    my $self = shift;

    my @proxies = @{$self->proxy_list()};
    my $current = $self->current_proxy();

    if ( $current == $#proxies ) {
        return 0;
    }
    else {
        $self->current_proxy(++$current);
        my $next_proxy = $self->proxy_list()->[$current];
        $self->eis_connector()->proxy($next_proxy);
        return 1;
    }
}

#### misc ####

# Create new SOAP EIS conector.
sub create_eis_connector
{
    my $uri   = shift;
    my $proxy = shift;

    my $sl = SOAP::Lite
        -> uri($uri)
        -> proxy($proxy);

    return $sl;
}

# Retrieve SOAP fault message.
sub get_soap_fault_message
{
    my $faulty_response = shift;
    my $fault_msg = join(', ', $faulty_response->faultcode(),
                               $faulty_response->faultstring(),
                               $faulty_response->faultdetail());
    return $fault_msg;
}

####

1; # needed by "use" or "require"
