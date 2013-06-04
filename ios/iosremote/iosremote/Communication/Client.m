//
//  Client.m
//  sdremote
//
//  Created by Liu Siqi on 6/3/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "Client.h"
#import "Server.h"
#import "Receiver.h"
#import "CommunicationManager.h"

@interface Client() <NSStreamDelegate>

@property (nonatomic, strong) NSInputStream* mInputStream;
@property (nonatomic, strong) NSOutputStream* mOutputStream;

@property (nonatomic, strong) NSString* mPin;
@property (nonatomic, strong) NSString* mName;
@property uint mPort;

@property (nonatomic, weak) Server* mServer;
@property (nonatomic, weak) Receiver* mReceiver;
@property (nonatomic, weak) CommunicationManager* mComManager;

@property (nonatomic, retain) NSMutableData* mData;

@property BOOL mReady;

@end



@implementation Client

@synthesize mInputStream = _mInputStream;
@synthesize mOutputStream = _mOutputStream;
@synthesize mPin = _mPin;
@synthesize mName = _mName;
@synthesize mServer = _mServer;
@synthesize mComManager = _mComManager;
@synthesize mData = _mData;
@synthesize mReady = _mReady;

NSString * const CHARSET = @"UTF-8";

- (id) initWithServer:(Server*)server
            managedBy:(CommunicationManager*)manager
        interpretedBy:(Receiver*)receiver
{
    self.mPin = @"";
    self.mName = server.serverName;
    self.mComManager = manager;
    self.mReceiver = receiver;
    // hardcoded here to test the communication TODO
    self.mPort = 1599;
    
    return self;
}

- (void)streamOpenWithIp:(NSString *)ip withPortNumber:(uint)portNumber
{
    NSLog(@"Connecting to %@:%u", ip, portNumber);
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, (__bridge CFStringRef)ip, portNumber, &readStream, &writeStream);
    
    if(readStream && writeStream)
    {
        CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        
        //Setup mInputStream
        self.mInputStream = (__bridge NSInputStream *)readStream;
        [self.mInputStream setDelegate:self];
        [self.mInputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [self.mInputStream open];
        
        //Setup outputstream
        self.mOutputStream = (__bridge NSOutputStream *)writeStream;
        [self.mOutputStream setDelegate:self];
        [self.mOutputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [self.mOutputStream open];
    }
    NSLog(@"Connected");
}

- (void) sendCommand:(NSString *)aCommand
{
    // UTF-8 as speficied in specification
    NSData * data = [aCommand dataUsingEncoding:NSUTF8StringEncoding];
    
    [self.mOutputStream write:(uint8_t *)[data bytes] maxLength:[data length]];
}

- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode {
    
    switch(eventCode) {
        case NSStreamEventHasBytesAvailable:
        {
            if(!self.mData) {
                self.mData = [NSMutableData data];
            }
            uint8_t buf[1024];
            unsigned int len = 0;
            len = [(NSInputStream *)stream read:buf maxLength:1024];
            if(len) {
                [self.mData appendBytes:(const void *)buf length:len];
                int bytesRead = 0;
                // bytesRead is an instance variable of type NSNumber.
                bytesRead += len;
            } else {
                NSLog(@"No data but received event for whatever reasons!");
            }
            
            NSString *str = [[NSString alloc] initWithData:self.mData
                                                  encoding:NSUTF8StringEncoding];
            NSLog(@"Data Received: %@", str);
            
            self.mData = nil;
        } break;
        default:
        {
        
        }
    
    }
}


- (void) connect
{
    [self streamOpenWithIp:self.mServer.serverAddress withPortNumber:self.mPort];
}



@end
